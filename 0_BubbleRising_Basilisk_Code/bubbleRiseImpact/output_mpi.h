

/**
## *dump()*: Basilisk snapshots

This function (together with *restore()*) can be used to dump/restore
entire simulations.

The arguments and their default values are:

*file*
: the name of the file to write to (mutually exclusive with *fp*). The
default is "dump".

*list*
: a list of scalar fields to write. Default is *all*.

*fp*
: a file pointer. Default is stdout.

*unbuffered*
: whether to use a file buffer. Default is false.
*/

@ if !_MPI
    trace void
    dump_test(struct Dump p)
{
  FILE *fp = p.fp;
  char def[] = "dump", *file = p.file ? p.file : p.fp ? NULL
                                                      : def;

  char *name = NULL;
  if (file)
  {
    name = (char *)malloc(strlen(file) + 2);
    strcpy(name, file);
    if (!p.unbuffered)
      strcat(name, "~");
    if ((fp = fopen(name, "w")) == NULL)
    {
      perror(name);
      exit(1);
    }
  }
  assert(fp);

  scalar *dlist = dump_list(p.list ? p.list : all);
  scalar size[];
  scalar *list = list_concat({size}, dlist);
  free(dlist);
  struct DumpHeader header = {t, list_len(list), iter, depth(), npe(),
                              dump_version};
  dump_header(fp, &header, list);

  subtree_size(size, false);

  foreach_cell()
  {
    unsigned flags = is_leaf(cell) ? leaf : 0;
    if (fwrite(&flags, sizeof(unsigned), 1, fp) < 1)
    {
      perror("dump(): error while writing flags");
      exit(1);
    }
    for (scalar s in list)
      if (fwrite(&s[], sizeof(double), 1, fp) < 1)
      {
        perror("dump(): error while writing scalars");
        exit(1);
      }
    if (is_leaf(cell))
      continue;
  }

  free(list);
  if (file)
  {
    fclose(fp);
    if (!p.unbuffered)
      rename(name, file);
    free(name);
  }
}
@ else // _MPI
    trace void
    dump_test(struct Dump p)
{
  FILE *fp = p.fp;
  char def[] = "dump", *file = p.file ? p.file : p.fp ? NULL
                                                      : def;

  if (fp != NULL || file == NULL)
  {
    fprintf(ferr, "dump(): must specify a file name when using MPI\n");
    exit(1);
  }

  char name[strlen(file) + 2];
  strcpy(name, file);
  if (!p.unbuffered)
    strcat(name, "~");
  FILE *fh = fopen(name, "w");
  if (fh == NULL)
  {
    perror(name);
    exit(1);
  }

  scalar *dlist = dump_list(p.list ? p.list : all);
  scalar size[];
  scalar *list = list_concat({size}, dlist);
  free(dlist);
  struct DumpHeader header = {t, list_len(list), iter, depth(), npe(),
                              dump_version};

#if MULTIGRID_MPI
  for (int i = 0; i < dimension; i++)
    (&header.n.x)[i] = mpi_dims[i];
  MPI_Barrier(MPI_COMM_WORLD);
#endif

  if (pid() == 0)
    dump_header(fh, &header, list);

  scalar index = {-1};

  index = new scalar;
  z_indexing(index, false);
  int cell_size = sizeof(unsigned) + header.len * sizeof(double);
  int sizeofheader = sizeof(header) + 4 * sizeof(double);
  for (scalar s in list)
    sizeofheader += sizeof(unsigned) + sizeof(char) * strlen(s.name);
  long pos = pid() ? 0 : sizeofheader;

  subtree_size(size, false);

  foreach_cell()
  {
    // fixme: this won't work when combining MPI and mask()
    if (is_local(cell))
    {
      long offset = sizeofheader + index[] * cell_size;
      if (pos != offset)
      {
        fseek(fh, offset, SEEK_SET);
        pos = offset;
      }
      unsigned flags = is_leaf(cell) ? leaf : 0;
      fwrite(&flags, 1, sizeof(unsigned), fh);
      for (scalar s in list)
        fwrite(&s[], 1, sizeof(double), fh);
      pos += cell_size;
    }
    if (is_leaf(cell))
      continue;
  }

  delete ({index});

  free(list);
  fclose(fh);
  if (!p.unbuffered && pid() == 0)
    rename(name, file);
}
@endif // _MPI

    trace bool
    restore_test(struct Dump p)
{
  FILE *fp = p.fp;
  char *file = p.file;
  if (file && (fp = fopen(file, "r")) == NULL)
    return false;
  assert(fp);

  struct DumpHeader header;
  if (fread(&header, sizeof(header), 1, fp) < 1)
  {
    fprintf(ferr, "restore(): error: expecting header\n");
    exit(1);
  }

#if TREE
  init_grid(1);
  foreach_cell()
  {
    cell.pid = pid();
    cell.flags |= active;
  }
  tree->dirty = true;
#else // multigrid
#if MULTIGRID_MPI
  if (header.npe != npe())
  {
    fprintf(ferr,
            "restore(): error: the number of processes don't match:"
            " %d != %d\n",
            header.npe, npe());
    exit(1);
  }
  dimensions(header.n.x, header.n.y, header.n.z);
  double n = header.n.x;
  int depth = header.depth;
  while (n > 1)
    depth++, n /= 2;
  init_grid(1 << depth);
#else // !MULTIGRID_MPI
  init_grid(1 << header.depth);
#endif
#endif // multigrid

  bool restore_all = (p.list == all);
  scalar *list = dump_list(p.list ? p.list : all);
  if (header.version == 161020)
  {
    if (header.len - 1 != list_len(list))
    {
      fprintf(ferr,
              "restore(): error: the list lengths don't match: "
              "%ld (file) != %d (code)\n",
              header.len - 1, list_len(list));
      exit(1);
    }
  }
  else
  { // header.version != 161020
    if (header.version != dump_version)
    {
      fprintf(ferr,
              "restore(): error: file version mismatch: "
              "%d (file) != %d (code)\n",
              header.version, dump_version);
      exit(1);
    }

    scalar *input = NULL;
    for (int i = 0; i < header.len; i++)
    {
      unsigned len;
      if (fread(&len, sizeof(unsigned), 1, fp) < 1)
      {
        fprintf(ferr, "restore(): error: expecting len\n");
        exit(1);
      }
      char name[len + 1];
      if (fread(name, sizeof(char), len, fp) < 1)
      {
        fprintf(ferr, "restore(): error: expecting s.name\n");
        exit(1);
      }
      name[len] = '\0';

      if (i > 0)
      { // skip subtree size
        bool found = false;
        for (scalar s in list)
          if (!strcmp(s.name, name))
          {
            input = list_append(input, s);
            found = true;
            break;
          }
        if (!found)
        {
          if (restore_all)
          {
            scalar s = new scalar;
            free(s.name);
            s.name = strdup(name);
            input = list_append(input, s);
          }
          else
            input = list_append(input, (scalar){INT_MAX});
        }
      }
    }
    free(list);
    list = input;

    double o[4];
    if (fread(o, sizeof(double), 4, fp) < 4)
    {
      fprintf(ferr, "restore(): error: expecting coordinates\n");
      exit(1);
    }
    origin(o[0], o[1], o[2]);
    size(o[3]);
  }

#if MULTIGRID_MPI
  long cell_size = sizeof(unsigned) + header.len * sizeof(double);
  long offset = pid() * ((1 << dimension * (header.depth + 1)) - 1) /
                ((1 << dimension) - 1) * cell_size;
  if (fseek(fp, offset, SEEK_CUR) < 0)
  {
    perror("restore(): error while seeking");
    exit(1);
  }
#endif // MULTIGRID_MPI

  scalar *listm = is_constant(cm) ? NULL : (scalar *){fm};
#if TREE && _MPI
  restore_mpi_test(fp, list);
#else
  foreach_cell()
  {
    unsigned flags;
    if (fread(&flags, sizeof(unsigned), 1, fp) != 1)
    {
      fprintf(ferr, "restore(): error: expecting 'flags'\n");
      exit(1);
    }
    // skip subtree size
    fseek(fp, sizeof(double), SEEK_CUR);
    for (scalar s in list)
    {
      double val;
      if (fread(&val, sizeof(double), 1, fp) != 1)
      {
        fprintf(ferr, "restore(): error: expecting a scalar\n");
        exit(1);
      }
      if (s.i != INT_MAX)
        s[] = val;
    }
    if (!(flags & leaf) && is_leaf(cell))
      refine_cell(point, listm, 0, NULL);
    if (is_leaf(cell))
      continue;
  }
  for (scalar s in all)
    s.dirty = true;
#endif

  scalar *other = NULL;
  for (scalar s in all)
    if (!list_lookup(list, s) && !list_lookup(listm, s))
      other = list_append(other, s);
  reset(other, 0.);
  free(other);

  free(list);
  if (file)
    fclose(fp);

  // the events are advanced to catch up with the time
  while (iter < header.i && events(false))
    iter = inext;
  events(false);
  while (t < header.t && events(false))
    t = tnext;
  t = header.t;
  events(false);

  return true;
}

void restore_mpi_test(FILE *fp, scalar *list1)
{
  long index = 0, nt = 0, start = ftell(fp);
  scalar size[], *list = list_concat({size}, list1);
  ;
  long offset = sizeof(double) * list_len(list);

  // read local cells
  static const unsigned short set = 1 << user;
  scalar *listm = is_constant(cm) ? NULL : (scalar *){fm};
  foreach_cell() 
    if (balanced_pid(index, nt, npe()) <= pid())
    {
      unsigned flags;
      if (fread(&flags, sizeof(unsigned), 1, fp) != 1)
      {
        fprintf(stderr, "restore(): error: expecting 'flags'\n");
        exit(1);
      }
      for (scalar s in list)
      {
        double val;
        if (fread(&val, sizeof(double), 1, fp) != 1)
        {
          fprintf(stderr, "restore(): error: expecting scalar\n");
          exit(1);
        }
        if (s.i != INT_MAX)
          s[] = val;
      }
      if (level == 0)
        nt = size[];
      cell.pid = balanced_pid(index, nt, npe());
      cell.flags |= set;
      if (!(flags & leaf) && is_leaf(cell))
      {
        if (balanced_pid(index + size[] - 1, nt, npe()) < pid())
        {
          fseek(fp, (sizeof(unsigned) + offset) * (size[] - 1), SEEK_CUR);
          index += size[];
          continue;
        }
        refine_cell(point, listm, 0, NULL);
      }
      index++;
      if (is_leaf(cell))
        continue;
    }

  // read non-local neighbors
  fseek(fp, start, SEEK_SET);
  index = 0;

  foreach_cell()
  {
    unsigned flags;
    if (fread(&flags, sizeof(unsigned), 1, fp) != 1)
    {
      fprintf(stderr, "restore(): error: expecting 'flags'\n");
      exit(1);
    }
    if (cell.flags & set)
      fseek(fp, offset, SEEK_CUR);
    else
    {
      for (scalar s in list)
      {
        double val;
        if (fread(&val, sizeof(double), 1, fp) != 1)
        {
          fprintf(stderr, "restore(): error: expecting a scalar\n");
          exit(1);
        }
        if (s.i != INT_MAX)
          s[] = val;
      }
      cell.pid = balanced_pid(index, nt, npe());
      if (is_leaf(cell) && cell.neighbors)
      {
        int pid = cell.pid;
        foreach_child()
            cell.pid = pid;
      }
    }
    if (!(flags & leaf) && is_leaf(cell))
    {
      bool locals = false;
      foreach_neighbor(1) if ((cell.flags & set) && (is_local(cell) || is_root(point)))
      {
        locals = true;
        break;
      }
      if (locals)
        refine_cell(point, listm, 0, NULL);
      else
      {
        fseek(fp, (sizeof(unsigned) + offset) * (size[] - 1), SEEK_CUR);
        index += size[];
        continue;
      }
    }
    index++;
    if (is_leaf(cell))
      continue;
  }

  /* set active flags */
  foreach_cell_post(is_active(cell))
  {
    cell.flags &= ~set;
    if (is_active(cell))
    {
      if (is_leaf(cell))
      {
        if (cell.neighbors > 0)
        {
          int pid = cell.pid;
          foreach_child()
              cell.pid = pid;
        }
        if (!is_local(cell))
          cell.flags &= ~active;
      }
      else if (!is_local(cell))
      {
        bool inactive = true;
        foreach_child() if (is_active(cell))
        {
          inactive = false;
          break;
        }
        if (inactive)
          cell.flags &= ~active;
      }
    }
  }

  flag_border_cells();

  mpi_boundary_update_test(list);

  free(list);
}

void mpi_boundary_update_test (scalar * list)
{
  mpi_boundary_update_buffers();
//   for (scalar s in list){
//     // s.dirty = true;
//   }
    
  grid->tn = 0; // so that tree is not "full" for the call below
  boundary (list);
  while (balance());

}