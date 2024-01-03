t = 4;
W = 190;
Front_W = W + t*2;
H = 100;

Angle_W = 12;
Large_Angle_L = W - 2*Angle_W - 2;
Small_Angle_L = H - 2*t - 2*Angle_W - 2;

Speaker_Dist_Between_Holes = 63;

pot_dia = 7+0.5; // volume control
jack_dia = 6; // 3.5 mm jack
jack_nut_dia = 12;
jack_nut_height = t-1.2;
button_dia = 7+0.5; // button
button_offset = 17;
toggle_dia = 6+0.5; // toggle switch
corner_dia = 7;
screw_dia = 4;
screw_offset = t*3;
screw_extra_offset = 20;
screw2_dia = 7;
screw2_height = 2;
nut_dia = 6.5;
nut_height = 1.5;
eps = 0.01;

main_enc_dia = 21+0.5;
main_enc_screws_dia = 15;
main_enc_screws_dist = 7;
main_enc_rotate_delta = 12;
main_enc_screw_dia = 3.5;
multi_enc_dia = 7+0.5; // CLAR pot

// LCD info
LCD_W = 58;
LCD_H = 32;
lcd_offset_y = 8;
lcd_offset_x = 6;
lcd_h = LCD_H - lcd_offset_x*2;
lcd_w = LCD_W - lcd_offset_y*2;

hole_dia = 3;
hole_offset_x = 1+hole_dia/2;
hole_offset_y = 1+hole_dia/2;

module so239() { // two holes
    front_d = 16;
    front_off = 18.8;
    front_hole_d = 3.2;

    rotate([0, 45, 0]) {
        translate([0, 0, 0]) rotate([90,0,0])
            cylinder(d=front_d,h=20, $fn=100, center=true);

        translate([-front_off/2, 0, -front_off/2]) rotate([90,0,0])
            cylinder(d=front_hole_d,h=20,$fn=100, center=true);

        translate([front_off/2, 0, front_off/2]) rotate([90,0,0])
            cylinder(d=front_hole_d,h=20,$fn=100, center=true);
    }
}

module speaker() {
    union() {
        translate([Speaker_Dist_Between_Holes/2, -Speaker_Dist_Between_Holes/2, 0]) {
            cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
            translate([0, 0, -(t-screw2_height)/2-eps])
                cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
        }
                
        translate([-Speaker_Dist_Between_Holes/2, Speaker_Dist_Between_Holes/2, 0]) {
            cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
            translate([0, 0, -(t-screw2_height)/2-eps])
                cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
        }
                
        translate([10, 20, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
        translate([0, 20, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
        translate([-10, 20, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
        
        translate([10, 10, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
        translate([10, 0, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
        translate([10, -10, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
        
        translate([20, 10, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
        translate([20, 0, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
        translate([20, -10, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);

        translate([0, 0, 0]) cylinder(d = screw_dia, h = 20, $fn = 200, center = true);
        translate([0, -10, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
        translate([0, 10, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
        
        translate([-10, 10, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
        translate([-10, 0, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
        translate([-10, -10, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
        
        translate([-20, 10, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
        translate([-20, 0, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
        translate([-20, -10, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true); 
        
        translate([10, -20, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
        translate([0, -20, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
        translate([-10, -20, 0]) cylinder(d = screw_dia, h = 20, $fn = 20, center = true);
    }
}

module front_back_base() {
    difference() {
       cube([Front_W, H, t], center = true);
        
        // screws
        translate([-Front_W/2+screw_offset+screw_extra_offset, H/2-screw_offset, 0]) {
            cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
            translate([0, 0, (t-screw2_height)/2+eps])
                cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
        } 
        
        translate([-Front_W/2+screw_offset+screw_extra_offset, -H/2+screw_offset, 0]) {
            cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
            translate([0, 0, (t-screw2_height)/2+eps])
                cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
        }
        
        translate([Front_W/2-screw_offset-screw_extra_offset, H/2-screw_offset, 0]) {
            cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
            translate([0, 0, (t-screw2_height)/2+eps])
                cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
        }
        
        translate([Front_W/2-screw_offset-screw_extra_offset, -H/2+screw_offset, 0]) {
            cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
            translate([0, 0, (t-screw2_height)/2+eps])
                cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
        }
        
        // center screws
       translate([-Front_W/2+screw_offset, 0, 0]) {
            cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
            translate([0, 0, (t-screw2_height)/2+eps])
                cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
        }
        translate([Front_W/2-screw_offset, 0, 0]) {
            cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
            translate([0, 0, (t-screw2_height)/2+eps])
                cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
        } 
    }
}


module front() {
    difference() {
        front_back_base();
        
        // volume control and RFAMP
        translate([-72, 20, 0]) {
            cylinder(d = pot_dia, h = t*2, $fn = 50, center = true);
            translate([30, 0, 0])
                cylinder(d = toggle_dia, h = t*2, $fn = 50, center = true);
        }
        
        // MODE and CLAR
        translate([72, 20, 0]) {
            cylinder(d = multi_enc_dia, h = t*2, $fn = 50, center = true);
            translate([-30, 0, 0])
                cylinder(d = toggle_dia, h = t*2, $fn = 50, center = true);
        }
        
        // LCD
        translate([0, 20, 0]) rotate([0, 0, 90]) {
            cube([lcd_h, lcd_w, t*2], center=true);
            translate([-LCD_H/2+hole_offset_x, LCD_W/2-hole_offset_y, 0]) {
                cylinder(h = t*2, d = hole_dia, center = true, $fn = 25);
                translate([0, 0, (t-screw2_height)/2+eps])
                    cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
            }
            translate([LCD_H/2-hole_offset_x, -LCD_W/2+hole_offset_y, 0]) {
                cylinder(h = t*2, d = hole_dia, center = true, $fn = 25);
                translate([0, 0, (t-screw2_height)/2+eps])
                    cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
            }
        }
        
        // 3.5 mm jacks
        translate([-57, -25, 0]) {
            translate([20, 0, 0]) {
                cylinder(d = jack_dia, h = t*2, $fn = 50, center = true);
                translate([0, 0, (t - jack_nut_height)/2+eps])
                    cylinder(d = jack_nut_dia, h = jack_nut_height, $fn = 50, center = true);
            }
            translate([0, 0, 0]) {
                cylinder(d = jack_dia, h = t*2, $fn = 50, center = true);
                translate([0, 0, (t - jack_nut_height)/2+eps])
                    cylinder(d = jack_nut_dia, h = jack_nut_height, $fn = 50, center = true);
            }
            translate([-20, 0, 0]) {
                cylinder(d = jack_dia, h = t*2, $fn = 50, center = true);
                translate([0, 0, (t - jack_nut_height)/2+eps])
                    cylinder(d = jack_nut_dia, h = jack_nut_height, $fn = 50, center = true);
            }
        }
        
        // Main dial
        translate([0, -20, 0]) {
            cylinder(d = main_enc_dia, h = t*2, $fn = 50, center = true);
            
            rotate([0,0,120*0 + main_enc_rotate_delta]) {
                translate([main_enc_screws_dist/2, main_enc_screws_dia, 0]) {
                    cylinder(d = main_enc_screw_dia, h = t*2, $fn = 25, center = true);
                    translate([0, 0, (t-screw2_height)/2+eps])
                        cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
                }
            }
            
            rotate([0,0,120*1 + main_enc_rotate_delta]) {
                translate([main_enc_screws_dist/2, main_enc_screws_dia, 0]) {
                    cylinder(d = main_enc_screw_dia, h = t*2, $fn = 25, center = true);
                    translate([0, 0, (t-screw2_height)/2+eps])
                        cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
                }
            }
            
            rotate([0,0,120*2 + main_enc_rotate_delta]) {
                translate([main_enc_screws_dist/2, main_enc_screws_dia, 0]) {
                    cylinder(d = main_enc_screw_dia, h = t*2, $fn = 25, center = true);
                    translate([0, 0, (t-screw2_height)/2+eps])
                        cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
                }
            }
        }
        
        // Buttons
        translate([55, -18, 0]) {
            translate([-button_offset, -button_offset/2, 0]) {
                cylinder(d = button_dia, h = t*2, $fn = 50, center = true);
                
                translate([0, button_offset, 0])
                    cylinder(d = button_dia, h = t*2, $fn = 50, center = true);
            }
            
            translate([0, -button_offset/2, 0]) {
                cylinder(d = button_dia, h = t*2, $fn = 50, center = true);
                
                translate([0, button_offset, 0])
                    cylinder(d = button_dia, h = t*2, $fn = 50, center = true);
            }
            
            translate([button_offset, -button_offset/2, 0]) {
                cylinder(d = button_dia, h = t*2, $fn = 50, center = true);
                
                translate([0, button_offset, 0])
                    cylinder(d = button_dia, h = t*2, $fn = 50, center = true);
            }
        }
    }
}

module side() {
    translate([W/2+t/2, 0, -W/2-t/2]) {
        difference() {
            cube([t, H, W], center = true);
            
            translate([0, H/2-screw_offset, -Front_W/2+screw_offset+screw_extra_offset]) {
                rotate([0, 90, 0]) {
                    cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                    translate([0, 0, (t-screw2_height)/2+eps])
                        cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
                }
            }
            
            translate([0, -H/2+screw_offset, -Front_W/2+screw_offset+screw_extra_offset]) {
                rotate([0, 90, 0]) {
                    cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                    translate([0, 0, (t-screw2_height)/2+eps])
                        cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
                }
            }
            
            translate([0, screw_extra_offset, -Front_W/2+screw_offset]) {
                rotate([0, 90, 0]) {
                    cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                    translate([0, 0, (t-screw2_height)/2+eps])
                        cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
                }
            }

            translate([0, -screw_extra_offset, -Front_W/2+screw_offset]) {
                rotate([0, 90, 0]) {
                    cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                    translate([0, 0, (t-screw2_height)/2+eps])
                        cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
                }
            }
            
            translate([0, H/2-screw_offset, Front_W/2-screw_offset-screw_extra_offset]) {
                rotate([0, 90, 0]) {
                    cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                    translate([0, 0, (t-screw2_height)/2+eps])
                        cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
                }
            }
            
            translate([0, -H/2+screw_offset, Front_W/2-screw_offset-screw_extra_offset]) {
                rotate([0, 90, 0]) {
                    cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                    translate([0, 0, (t-screw2_height)/2+eps])
                        cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
                }
            }
            
            translate([0, screw_extra_offset, Front_W/2-screw_offset]) {
                rotate([0, 90, 0]) {
                    cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                    translate([0, 0, (t-screw2_height)/2+eps])
                        cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
                }
            }
            
             translate([0, -screw_extra_offset, Front_W/2-screw_offset]) {
                rotate([0, 90, 0]) {
                    cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                    translate([0, 0, (t-screw2_height)/2+eps])
                        cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
                }
            }
        }
    }
}

module top_bottom_base() {
    difference() {
        cube([W, t, W], center = true);
        
        translate([-Front_W/2+screw_offset+screw_extra_offset/2, 0,  Front_W/2-screw_offset]) {
            rotate([90, 0, 0]) {
                cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                translate([0, 0, (t-screw2_height)/2+eps])
                    cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
            }
        }
        
        translate([Front_W/2-screw_offset-screw_extra_offset/2, 0,  Front_W/2-screw_offset]) {
            rotate([90, 0, 0]) {
                cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                translate([0, 0, (t-screw2_height)/2+eps])
                    cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
            }
        }
        
        translate([Front_W/2-screw_offset, 0,  -Front_W/2+screw_offset+screw_extra_offset/2]) {
            rotate([90, 0, 0]) {
                cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                translate([0, 0, (t-screw2_height)/2+eps])
                    cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
            }
        }
        
        translate([Front_W/2-screw_offset, 0,  Front_W/2-screw_offset-screw_extra_offset/2]) {
            rotate([90, 0, 0]) {
                cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                translate([0, 0, (t-screw2_height)/2+eps])
                    cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
            }
        }
        
        translate([-Front_W/2+screw_offset+screw_extra_offset/2, 0,  -Front_W/2+screw_offset]) {
            rotate([90, 0, 0]) {
                cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                translate([0, 0, (t-screw2_height)/2+eps])
                    cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
            }
        }
        
        translate([Front_W/2-screw_offset-screw_extra_offset/2, 0,  -Front_W/2+screw_offset]) {
            rotate([90, 0, 0]) {
                cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                translate([0, 0, (t-screw2_height)/2+eps])
                    cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
            }
        }
        
        translate([-Front_W/2+screw_offset, 0,  -Front_W/2+screw_offset+screw_extra_offset/2]) {
            rotate([90, 0, 0]) {
                cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                translate([0, 0, (t-screw2_height)/2+eps])
                    cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
            }
        }
        
        translate([-Front_W/2+screw_offset, 0,  Front_W/2-screw_offset-screw_extra_offset/2]) {
            rotate([90, 0, 0]) {
                cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                translate([0, 0, (t-screw2_height)/2+eps])
                    cylinder(d = screw2_dia, h = screw2_height+eps, $fn = 25, center = true);
            }
        }
    }

}

module bottom() {
    translate([0, -H/2+t/2, -W/2-t/2]) {
        difference() {
            top_bottom_base();
            
            translate([-Front_W/2+25, 0,  -Front_W/2+25]) {
                rotate([90, 0, 0]) {
                    cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                }
            }
            
            translate([Front_W/2-25, 0,  -Front_W/2+25]) {
                rotate([90, 0, 0]) {
                    cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                }
            }
            
            translate([-Front_W/2+25, 0,  Front_W/2-25]) {
                rotate([90, 0, 0]) {
                    cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                }
            }
            
            translate([Front_W/2-25, 0,  Front_W/2-25]) {
                rotate([90, 0, 0]) {
                    cylinder(d = screw_dia, h = t*2, $fn = 25, center = true);
                }
            }
        }
    }
}

module large_angle() {
    difference() {
        union() {
            translate([0, -H/2+t*1.5, -Angle_W/2-t/2])
                cube([Large_Angle_L, t, Angle_W], center=true);
            translate([0, -H/2+t+Angle_W/2, -t])
                cube([Large_Angle_L, Angle_W, t], center=true);
        }
    
        translate([0, -H/2+t+Angle_W/2+t, -t]) {
           cube([Large_Angle_L/2, Angle_W, t*2], center=true);
        }
        
        translate([-Front_W/2+screw_offset+screw_extra_offset, -H/2+screw_offset, 0]) {
            cylinder(d = screw_dia, h = Angle_W*2, $fn = 25, center = true);
            translate([0, 0, -t-(t-nut_height)/2])
                cylinder(d = nut_dia, h = nut_height+eps, $fn = 25, center = true);
        }
        
        translate([Front_W/2-screw_offset-screw_extra_offset, -H/2+screw_offset, 0]) {
            cylinder(d = screw_dia, h = Angle_W*2, $fn = 25, center = true);
            translate([0, 0, -t-(t-nut_height)/2])
                cylinder(d = nut_dia, h = nut_height+eps, $fn = 25, center = true);
        }
        
        translate([Front_W/2-screw_offset-screw_extra_offset/2, -H/2,  (-t/2-W/2)+Front_W/2-screw_offset]) {
            rotate([90, 0, 0]) {
                cylinder(d = screw_dia, h = Angle_W*2, $fn = 25, center = true);
                translate([0, 0, -t*2+nut_height/2-eps])
                    cylinder(d = nut_dia, h = nut_height+eps, $fn = 25, center = true);
            }
        }
        
        translate([-Front_W/2+screw_offset+screw_extra_offset/2, -H/2,  (-t/2-W/2)+Front_W/2-screw_offset]) {
            rotate([90, 0, 0]) {
                cylinder(d = screw_dia, h = Angle_W*2, $fn = 25, center = true);
                translate([0, 0, -t*2+nut_height/2-eps])
                    cylinder(d = nut_dia, h = nut_height+eps, $fn = 25, center = true);
            }
        }
    }
}

module small_angle() {
    difference() {
        translate([W/2-t/2, 0, -Angle_W/2-t/2]) {
            union() {
                cube([t, Small_Angle_L, Angle_W], center = true);
                translate([-Angle_W/2+t/2, 0, Angle_W/2-t/2]) rotate([0, 90, 0])
                    cube([t, Small_Angle_L, Angle_W], center = true);
            }
        }
    
        translate([W/2+t/2, 0, -W/2-t/2]) {
            translate([0, screw_extra_offset, Front_W/2-screw_offset]) {
                rotate([0, 90, 0]) {
                    cylinder(d = screw_dia, h = Angle_W*2, $fn = 25, center = true);
                    translate([0, 0, -t*1.5+nut_height/2-eps])
                        cylinder(d = nut_dia, h = nut_height+eps, $fn = 25, center = true);
                }
            }
            
             translate([0, -screw_extra_offset, Front_W/2-screw_offset]) {
                rotate([0, 90, 0]) {
                    cylinder(d = screw_dia, h = Angle_W*2, $fn = 25, center = true);
                    translate([0, 0, -t*1.5+nut_height/2-eps])
                        cylinder(d = nut_dia, h = nut_height+eps, $fn = 25, center = true);
                }
            }
        }
        
        translate([Front_W/2-screw_offset, 0, 0]) {
            cylinder(d = screw_dia, h = Angle_W*2, $fn = 25, center = true);
            translate([0, 0, -t*1.5+nut_height/2-eps])
                cylinder(d = nut_dia, h = nut_height+eps, $fn = 25, center = true);
        } 
    }
}

module back() {
    translate([0, 0, -W-t]) difference() {
        rotate([0, 180, 0]) front_back_base();
        // UHF connecter
        translate([W/2-30, -H/4+5, 0]) rotate([90, 0, 0]) so239();
        // DC Jack
        translate([-W/2+30, -H/4+5, 0]) cylinder(d = 12.5, h = 20, $fn = 100, center = true);
        // Ground screw
        translate([0, -H/4+5, 0]) cylinder(d = 6.5, h = 20, $fn = 100, center = true);
    }
}

module top() {
    translate([0, H/2-t/2, -W/2-t/2]) {
        difference() {
            rotate([180, 0, 0]) top_bottom_base();
            
            translate([W/2-Speaker_Dist_Between_Holes/2-screw_extra_offset, 0, W/2-Speaker_Dist_Between_Holes/2-screw_extra_offset]) rotate([90, 0, 0]) {
                speaker();
            }
        }
    }
}

rotate([90, 0, 0]) translate([0, 0, W/2]) {
    front();
    back();
    bottom();
    %top();
    small_angle();
    large_angle();
    translate([Front_W/2-t/2, 0, -Front_W/2+t/2]) rotate([0, 90, 0]) 
        large_angle();
    side();
}