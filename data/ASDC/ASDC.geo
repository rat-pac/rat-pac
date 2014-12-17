{
// ASDCDetectorFactory recomputes the following geometry components to match
// the values specified in this table.
name: "ASDC_PARAMS",
valid_begin: [0, 0],
valid_end: [0, 0],

photocathode_coverage: 0.90,
veto_coverage: 0.00,

fiducial_diameter: 40000.0,
fiducial_height: 40000.0,
fiducial_buffer: 2000.0,

wall_thickness: 10.0,

veto_buffer: 2000.0,
veto_pmt_offset: 700.0,

}

{
name: "GEO",
index: "world",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "", // world volume has no mother
type: "box",
size: [30000.0, 30000.0, 30000.0], // mm, half-length
material: "rock",
invisible: 1,
}

{
name: "GEO",
index: "tank",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "world",
type: "tube",
r_max: 20010.0,
size_z: 20010.0,
position: [0.0, 0.0, 0.0],
material: "steel",
color: [1.0, 0.0, 0.0, 0.1],
drawstyle: "solid"
}

{
name: "GEO",
index: "detector",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "tank",
type: "tube",
r_max: 20000.0,
size_z: 20000.0,
position: [0.0, 0.0, 0.0],
material: "scintillator",
color: [0.4, 0.4, 0.6, 0.3],
drawstyle: "solid"
}

{ 
name: "GEO", 
index: "inner_pmts", 
enable: 1,
valid_begin: [0, 0], 
valid_end: [0, 0], 
mother: "detector", 
type: "pmtarray", 
max_pmts: 0, //index of last inner pmt
start_num: 0, //index of first inner pmt
pmt_type: "r11780_hqe",
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/inner", 
efficiency_correction: 1.000,  
pos_table: "PMTINFO", //generated on the fly
orientation: "manual",
} 

{ 
name: "GEO", 
index: "veto_pmts",
enable: 1,
valid_begin: [0, 0], 
valid_end: [0, 0], 
mother: "detector", 
type: "pmtarray", 
max_pmts: 0, //index of last veto pmt
start_num: 4330, //index of first veto pmt
pmt_type: "r11780_hqe",
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/veto", 
efficiency_correction: 1.000,  
pos_table: "PMTINFO", //generated on the fly
orientation: "manual",
} 


