{
name: "GEO",
index: "gdml_file",
valid_begin: [0, 0],
valid_end: [0, 0],
gdml_file: "lybox.gdml",
opdet_lv_name: "volSiPMbox",
}

{
name: "GEO",
index: "scint_sipm_surface",
valid_begin: [0, 0],
valid_end: [0,0],
mother:"scint",
type:"border",
volume1:"pvScint",
volume2:"SiPM0",
//surface:"stainless_steel",
surface:"chip_silicon",
reverse: 1,
}