{
  name:"GEO",
  valid_begin: [0,0],
  valid_end: [0,0],
  gdml_file: "nudotv0.gdml",
  opdet_lv_name: "volPC",
}

{
  name:"GEO",
  index: "border_tank_wall",
  valid_begin: [0,0],
  valid_end: [0,0],
  mother:"border",
  type:"border",
  volume1:"pvTank",
  volume2:"pvScint",
  surface:"stainless_steel",
  reverse:1,
}