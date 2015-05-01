// Simplified SNO detector geometry

{
name: "GEO",
index: "world",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "", // world volume has no mother
type: "box",
size: [20000.0, 20000.0, 20000.0], // mm, half-height
material: "rock",
color: [0.67, 0.29, 0.0],
invisible: 1,
}

{
name: "GEO",
index: "cavity",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "world",
type: "box",
size: [10000.0, 10000.0, 10000.0], // mm
material: "lightwater_sno",
color: [1.0, 1.0, 1.0],
invisible: 1,
}

{
name: "GEO",
index: "psup",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "cavity",
type: "sphere",
r_max: 9000.0, // Outer radius, mm
material: "stainless_steel",
color: [1.0, 1.0, 1.0],
invisible: 1,
}

{
name: "GEO",
index: "h2o",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "psup",
type: "sphere",
r_max: 8900.0, // Outer radius, mm
material: "lightwater_sno",
color: [0.7, 0.4, 0.5, 0.1],
}

{
name: "GEO",
index: "av",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "h2o",
type: "sphere",
r_max: 6060.0, // Outer radius, mm
material: "acrylic_sno",
color: [1.0, 1.0, 0.0, 0.1],
}

{
name: "GEO",
index: "innerPMT",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "h2o",
type: "pmtarray",
pmt_model: "r1408", 
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/inner",
efficiency_correction: 1.027,  
add_concentrator: 1, // Flag: 0 = no concentrators, 1 = concentrators
pos_table: "PMTINFO",
orientation: "point", // Aim all PMTs at a point
	     	      // "manual" means there is a dir_x, dir_y, dir_z
	              // in pos_table for manual orientation of PMTs
orient_point: [0.0, 0.0, 0.0],
}

{
name: "GEO",
index: "d2o",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "av",
type: "sphere",
r_max: 6005.0, // Outer radius, mm
material: "HeavyWater",
color: [0.0, 0.3, 1.0, 0.2],
}

