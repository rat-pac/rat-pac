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
type: "sphere",
r_max: 9000.0,
position: [0.0, 0.0, 0.0],
material: "water",
color: [0.4, 0.4, 0.6, 0.05],
}

{
name: "GEO",
index: "target",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "water",
type: "sphere",
r_max: 6000.0,
position: [0.0, 0.0, 0.0],
material: "water",
color: [0.4, 0.4, 0.6, 0.05],
}

{ 
name: "GEO", 
index: "pmts", 
valid_begin: [0, 0], 
valid_end: [0, 0], 
mother: "water", 
type: "pmtarray", 
pmt_model: "r11780_hqe", 
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/inner", 
pos_table: "PMTINFO", 
orientation: "point",
orient_point: [0.0, 0.0, 0.0], 
} 

