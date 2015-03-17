{
name: "GEO",
index: "world",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "", // world volume has no mother
type: "box",
size: [20000.0, 20000.0, 20000.0], // mm, half-length
material: "rock",
invisible: 1,
}

{
name: "GEO",
index: "water",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "world",
type: "tube",
r_max: 3500.0,
size_z: 3500.0,
position: [0.0, 0.0, 0.0],
material: "water",
color: [0.4, 0.4, 0.6, 0.1],
}

{ 
name: "GEO", 
index: "pmts", 
valid_begin: [0, 0], 
valid_end: [0, 0], 
mother: "water", 
type: "pmtarray", 
pmt_model: "r1408", 
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/inner", 
efficiency_correction: 1.027,  
pos_table: "PMTINFO", 
orientation: "manual",
//orient_point: [0.0, 0.0, 0.0], 
} 

