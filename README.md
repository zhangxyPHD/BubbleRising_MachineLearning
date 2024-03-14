# Machine learning enhanced exploration of bubble dynamics beneath horizontal wall
## 0_BubbleRising_Basilisk_Code
### Multi-task running scripts
run_script.sh
### basilisk code
<ol>
<li>bubbleRise.c :main program</li>
<ol>
<li>run_continure.sh :Resumption of calculations after interruption</li>
<li>run.sh :New calculation, which clears the result file</li>
</ol>
<li>Post-processing
<ol>
<li>dumpToVTU.c :Converting snap to a paraview file</li>
<li>run_vtu.sh :Load snap, batch generate vtu files</li>
<li>getResults.c :Based on the snap file, perform restore, then calculate the position, deformation, and velocity of the bubble</li>
<li>run_results.sh :Load snap, extract bubble results</li>
<li>plot_script.gp :Plotting with gnuplot</li>
</ol>

<li>Header file</li>
<ol>
<li>adapt_wavelet_limited.h :Used for grid refinement, based on the maximum level of the specified refinement area.</li>
<li>output_boundary.h :Calculate the boundary of the bubble based on the f-field and calculate the top, bottom, left, and right coordinates to compute the Ratio</li>
<li>output_vtu_foreach.h :Converting paraview files</li>
<li>output_mpi.h：Changed the original dump and restore files (conflicting with mpi), which can be run after the modifications, which are as follows, commenting out the last few lines(//s.dirty = true;)
</ol>
</ol>

### An example
Take Ga40-Bo0.1-Lrise1-Level6 as an example
<ol>
<li>model :Stores the basilisk source files corresponding to the different parameters.</li>
<li>results :Stores the result files corresponding to the different parameters</li>
<li>results_Picture :Stores the plots corresponding to the different parameters</li>
</ol>


## 1_BubbleRising_FeaturePre
<l>
<li>0_Data_pre :Extract data</li>
<li>1_Normalization :Training set preprocessing</li>
<li>2_Pre_Feature Character values prediction for bubble rise</li>
</ul>

All data are located in folder "a_Ga_Bo_results_level6".
All input and output data are summarised in file "a_data_Ga_Bo.csv".
