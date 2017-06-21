{
name: "GEO",
index: "world",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "",
type: "sphere",
r_max: 6000.0,
position: [0.0, 0.0, 0.0],
material: "air",
color: [1.0, 0.0, 0.0, 0.1],
drawstyle: "solid",
invisible: 1
}

{
name: "GEO",
index: "theGround",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "world",
type: "sphere",
r_max: 6000.0,
theta_start: 90.0,
theta_delta: 180.0,
phi_start: 0.0,
phi_delta: 360.0,
position: [0.0, 0.0, 0.0],
material: "rock",
color: [1.0, 0.0, 0.0, 0.1],
drawstyle: "solid"
invisible: 1
}

{
name: "GEO",
index: "tank", //this is the tank
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "world",
type: "tube",
r_max: 1980.2,
size_z: 1651.2,
position: [0.0, 0.0, 1651.2 ] , // position: [0.0, 0.0, 0.0],
material: "stainless_steel",
color: [1.0, 0.0, 0.0, 0.1],
drawstyle: "solid"
}

{
name: "GEO",
index: "liner",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "tank",
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
mother: "liner",
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
material: "doped_water_12",
color: [0.0, 1.0, 0.0, 0.1],
drawstyle: "solid"
}

{
name: "GEO",
index: "reflectiveTeflon",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "detector",
type: "tube",
r_max: 749.3,
size_z: 635.0,
position: [0.0, 0.0, 0.0],
material: "ptfe_90",
color: [0.0, 0.0, 1.0, 0.1],
drawstyle: "solid",
invisible: 1
}

//the mid is replaced with reflective teflon
/*
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
*/

{
name: "GEO",
index: "mid_water",
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "reflectiveTeflon", //mother: "mid",
type: "tube",
r_max: 748.3,
size_z: 634.0,
position: [0.0, 0.0, 0.0],
material: "doped_water_12",
color: [0.0, 0.0, 1.0, 0.1],
drawstyle: "solid"
}

{
//Bergevin: Set the interface were reflection can occur. Must make sure volume1 and volume2
//are in the correct order
name: "GEO",
index: "midsurface",
valid_begin: [0, 0],
valid_end: [0, 0],
invisible: 0, // omitted for visualization
mother: "reflectiveTeflon", //not used but needs to be a valid name, parent of 'a' and 'b' would be best choice
type: "border",
volume1: "mid_water",
volume2: "reflectiveTeflon",
reverse: 0, //0 only considers photons from a->b, 1 does both directions
surface: "ptfe_90",
}

{
name: "GEO",
index: "inner_pmts",
enable: 1,
valid_begin: [0, 0],
valid_end: [0, 0],
mother: "mid_water",
type: "pmtarray",
pmt_model: "r7081",
mu_metal: 1,
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/inner",
//3sign
efficiency_correction: 5.000000e-01 ,
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
pmt_model: "r7081",
mu_metal: 1,
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/inner",
//3sign
efficiency_correction: 5.000000e-01 ,
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
pmt_model: "r7081",
mu_metal: 1,
pmt_detector_type: "idpmt",
sensitive_detector: "/mydet/pmt/veto",
//3sign
efficiency_correction: 5.000000e-01 ,
pos_table: "PMTINFO_outerveto",
orientation: "manual",
}
