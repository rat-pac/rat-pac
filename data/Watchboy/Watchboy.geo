{
name: "GEO",
index: "world", //this is the tank
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "",
type: "tube",
r_max: 1980.2,
size_z: 1651.2,
position: [0.0, 0.0, 0.0],
material: "steel",
color: [1.0, 0.0, 0.0, 0.1],
drawstyle: "solid"
}

{
name: "GEO",
index: "liner",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "world",
type: "tube",
r_max: 1980.1,
size_z: 1651.1,
position: [0.0, 0.0, 0.0],
material: "polyethylene",
color: [1.0, 0.0, 0.0, 0.0],
drawstyle: "solid"
}

{
name: "GEO",
index: "detector", 
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "world",
type: "tube",
r_max: 1980.0,
size_z: 1651.0,
position: [0.0, 0.0, 0.0],
material: "water",
color: [1.0, 0.0, 0.0, 0.0],
drawstyle: "solid"
}

{
name: "GEO",
index: "top",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "detector",
type: "tube",
r_max: 749.3,
size_z: 508.0,
position: [0.0, 0.0, 1143.0],
material: "polyethylene",
color: [0.0, 1.0, 0.0, 0.1],
drawstyle: "solid"
}

{
name: "GEO",
index: "top_water",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "top",
type: "tube",
r_max: 748.3,
size_z: 507.0,
position: [0.0, 0.0, 0.0],
material: "water",
color: [0.0, 1.0, 0.0, 0.1],
drawstyle: "solid"
}

{
name: "GEO",
index: "mid",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "detector",
type: "tube",
r_max: 749.3,
size_z: 635.0,
position: [0.0, 0.0, 0.0],
material: "polyethylene",
color: [0.0, 0.0, 1.0, 0.1],
drawstyle: "solid"
}

{
name: "GEO",
index: "mid_water",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "mid",
type: "tube",
r_max: 748.3,
size_z: 634.0,
position: [0.0, 0.0, 0.0],
material: "water",
color: [0.0, 0.0, 1.0, 0.1],
drawstyle: "solid"
}

{
name: "GEO",
index: "calibTube",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "detector",
type: "tube",
r_min: 0.0,
r_max: 11.1,
size_z: 635.0,
position: [1314.5, 0.0,825.5],
material: "steel",
color: [0.0, 1.0, 1.0, 0.1],
drawstyle: "solid"
}

{
name: "GEO",
index: "calibAir",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "calibTube",
type: "tube",
r_min: 0.0,
r_max: 10.1,
size_z: 635.0,
position: [0.0,0.0,0.0],
material: "steel",
color: [0.0, 1.0, 1.0, 0.1],
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
pmt_model: "r7081_hqe",
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/inner", 
efficiency_correction: 1.000,  
pos_table: "PMTINFO_inner",
orientation: "manual",
} 

{
name: "GEO", 
index: "inner_veto", 
enable: 1,
valid_begin: [0, 0], 
valid_end: [0, 0], 
mother: "top_water", 
type: "pmtarray",
pmt_model: "r7081_hqe",
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/inner", 
efficiency_correction: 1.000,  
pos_table: "PMTINFO_innerveto",
orientation: "manual",
}

{ 
name: "GEO", 
index: "outer_veto",
enable: 1,
valid_begin: [0, 0], 
valid_end: [0, 0], 
mother: "detector", 
type: "pmtarray",
pmt_model:"r7081_hqe",
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/veto", 
efficiency_correction: 1.000,  
pos_table: "PMTINFO_outerveto", 
orientation: "manual",
} 



