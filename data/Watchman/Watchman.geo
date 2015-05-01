{
name: "GEO",
index: "world",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "", // world volume has no mother
type: "box",
size: [10000.0, 10000.0, 10000.0], // mm, half-length
material: "steel", //rock?
invisible: 1,
}

{
name: "GEO",
index: "tank",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "world",
type: "tube",
r_max: 8000.0,
size_z: 8000.0,
position: [0.0, 0.0, 0.0],
material: "steel",
color: [0.2,0.2,0.2,0.1],
drawstyle: "solid"
}

{
name: "GEO",
index: "detector",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "tank",
type: "tube",
r_max: 7984.125,
size_z: 7984.125, //half height, mm
position: [0.0, 0.0, 0.0],
material: "doped_water",
color: [0.2,0.2,0.2,0.1],
drawstyle: "solid"
}

{ //position table for hold-up cables
name: "cable_pos",
valid_begin: [0, 0],
valid_end: [0, 0],
x: [6406.35d,6392.63d,6351.54d,6283.25d,6188.06d,6066.37d,5918.7d,5745.68d,5548.06d,5326.69d,5082.5d,4816.55d,4529.97d,4224.d,3899.94d,3559.18d,3203.18d,2833.46d,2451.6d,2059.25d,1658.09d,1249.82d,836.196d,418.995d,0.d,-418.995d,-836.196d,-1249.82d,-1658.09d,-2059.25d,-2451.6d,-2833.46d,-3203.18d,-3559.18d,-3899.94d,-4224.d,-4529.97d,-4816.55d,-5082.5d,-5326.69d,-5548.06d,-5745.68d,-5918.7d,-6066.37d,-6188.06d,-6283.25d,-6351.54d,-6392.63d,-6406.35d,-6392.63d,-6351.54d,-6283.25d,-6188.06d,-6066.37d,-5918.7d,-5745.68d,-5548.06d,-5326.69d,-5082.5d,-4816.55d,-4529.97d,-4224.d,-3899.94d,-3559.18d,-3203.18d,-2833.46d,-2451.6d,-2059.25d,-1658.09d,-1249.82d,-836.196d,-418.995d,0.d,418.995d,836.196d,1249.82d,1658.09d,2059.25d,2451.6d,2833.46d,3203.18d,3559.18d,3899.94d,4224.d,4529.97d,4816.55d,5082.5d,5326.69d,5548.06d,5745.68d,5918.7d,6066.37d,6188.06d,6283.25d,6351.54d,6392.63d,],
y: [0.d,418.995d,836.196d,1249.82d,1658.09d,2059.25d,2451.6d,2833.46d,3203.18d,3559.18d,3899.94d,4224.d,4529.97d,4816.55d,5082.5d,5326.69d,5548.06d,5745.68d,5918.7d,6066.37d,6188.06d,6283.25d,6351.54d,6392.63d,6406.35d,6392.63d,6351.54d,6283.25d,6188.06d,6066.37d,5918.7d,5745.68d,5548.06d,5326.69d,5082.5d,4816.55d,4529.97d,4224.d,3899.94d,3559.18d,3203.18d,2833.46d,2451.6d,2059.25d,1658.09d,1249.82d,836.196d,418.995d,0.d,-418.995d,-836.196d,-1249.82d,-1658.09d,-2059.25d,-2451.6d,-2833.46d,-3203.18d,-3559.18d,-3899.94d,-4224.d,-4529.97d,-4816.55d,-5082.5d,-5326.69d,-5548.06d,-5745.68d,-5918.7d,-6066.37d,-6188.06d,-6283.25d,-6351.54d,-6392.63d,-6406.35d,-6392.63d,-6351.54d,-6283.25d,-6188.06d,-6066.37d,-5918.7d,-5745.68d,-5548.06d,-5326.69d,-5082.5d,-4816.55d,-4529.97d,-4224.d,-3899.94d,-3559.18d,-3203.18d,-2833.46d,-2451.6d,-2059.25d,-1658.09d,-1249.82d,-836.196d,-418.995d,],
z: [0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,],
dir_x: [0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,],
dir_y: [0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,0.d,],
dir_z: [1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,1.d,],
}

{
name: "GEO",
index: "cables",
enable: 1,
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "detector",
type: "tubearray",
r_max: 9.525,
size_z: 7984.125,
pos_table: "cable_pos",
orientation: "manual",
material: "steel",
drawstyle: "solid",
color: [0.2,0.2,0.2,0.0],
}

{
name: "inner_vis",
valid_begin: [0, 0],
valid_end: [0, 0],
color: [0.0, 0.5, 0.0, 0.2],
}

{
name: "veto_vis",
valid_begin: [0, 0],
valid_end: [0, 0],
color: [1.0, 0.0, 1.0, 0.3],
}

{
name: "GEO",
index: "shield",
enable: 1,
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "detector",
type: "watchmanshield", //see the geo factory

//builds pmt backs/covers
pmtinfo_table: "PMTINFO",
back_semi_x: 152.5,
back_semi_y: 152.5,
back_semi_z: 301.5, //this should perhaps be a little larger
back_thickness: 3.175,
back_material: "polypropylene",
inner_start: 0,
inner_len: 4330, //set to 0 to prevent building covers
inner_back_surface: "black_water",
inner_back_vis: "inner_vis",
veto_start: 4330,
veto_len: 482, //set to 0 to prevent building covers
veto_back_surface: "black_water",
veto_back_vis: "veto_vis",

//properties to define the shield
detector_size: 16000.0,
shield_thickness: 1600.0,
steel_thickness: 1.5875,
cols: 96,
rows: 30,
frame_material: "steel",
inside_surface: "black_water",
outside_surface: "white_water",

drawstyle: "solid",
position: [0.0, 0.0, 0.0],
color: [0.1, 0.1, 0.1, 0.1],
}

// Fiducial defined as separate geometry component a la baccarat
{
name: "GEO",
index: "fiducial",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "detector",
type: "tube",
r_max: 5400.0,
size_z: 5400.0,
position: [0.0, 0.0, 0.0],
material: "doped_water",
color: [0.2,0.2,0.2,0.1],
drawstyle: "solid",
invisible: 1
}

{ 
name: "GEO", 
index: "inner_pmts", 
enable: 1,
valid_begin: [0, 0], 
valid_end: [0, 0], 
mother: "detector", 
type: "pmtarray", 
end_idx: 4329, //idx of the last pmt
start_idx: 0, //idx of the first pmt
pmt_model: "r11780_hqe",
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/inner", 
efficiency_correction: 1.000,  
pos_table: "PMTINFO", //generated by positions.nb
orientation: "manual",
color: [0.3, 0.5, 0.0, 0.7],
} 

{ 
name: "GEO", 
index: "veto_pmts",
enable: 1,
valid_begin: [0, 0], 
valid_end: [0, 0], 
mother: "detector", 
type: "pmtarray", 
end_idx: 4811, //idx of the last pmt
start_idx: 4330, //idx of the first pmt
pmt_model: "r11780_hqe",
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/veto", 
efficiency_correction: 1.000,  
pos_table: "PMTINFO", //generated by positions.nb
orientation: "manual",
} 


