;;                                                               
;;                         turn                            fire  detection weapon
;;                 speed   speed    accel  cost   health  delay  range     type
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


(setf dependencies  
      '((trike_tech             tank_tech  explosives_tech)
        (missile_truck_tech     tank_tech  guided_missile_tech)
        (bomb_truck_tech        trike_tech explosives_tech)
        (eletric_car_tech       tank_tech  volt_tech)
        (tank_buster_tech       missile_truck_tech)
        (bridger_tech           engineer_tech)
        (helicopter_tech        aircraft_tech guided_missile_tech)
        (bomber_tech            aircraft_tech jet_tech explosives_tech)
        (jet_tech               jet_tech)
        (money_plane_tech       aircraft_tech)))



/*(research_items
 ; tech name         research-time   cost   call-got                   call-lost
 (bomb_tech          100             1000   (add_build bomb_truck))
 (construction_tech  100             1000                         ) */
        
;;                                                               
;;                         turn                            fire  detection weapon
;;                 speed   speed    accel  cost   health  delay  range     type
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defaults 
  (default         0.08    0.2    0.020     0    100      3      0         nil)
  (electric_car    0.08    0.2    0.020   800     50      0      3         electric_beam)
  (peon_tank       0.08    0.20   0.020   300     50     10      4         b90mm)
  (helicopter      0.08    0.8    0.020  1000     70     30      5         guided_missile);;health offset 20
  (jet             0.10    0.07   0.020  1500     70      0      5         chain_gun)
  (engineer        0.08    0.2    0.020   300     30      0      1         nil)
  (trike           0.08    0.2    0.020   500     80      0      5         trike)
  (tank_buster     0.08    0.2    0.020   800     30      0      18        buster_rocket)
  (rocket_tank     0.08    0.2    0.020   800     40     30      6         guided_missile)
  (bomb_truck      0.08    0.2    0.010   300    100      0      0         nil)
  (bridger         0.08    0.2    0.010   300     50      0      0         nil)
  (moneyplane      0.20    0.1    0.010   500     50      0      0         nil);;health offset=20
  (bomber          0.10    0.05   0.020  1500    100      2      6         dropped_bomb)
  (cobra_lisp      0.10    0.20   0.020  2000    500      2     10         heavy_rocket)
  (cobra_tank      0.10    0.20   0.015  2000    600      3	14         napalm)
  (bird            0.06    0.10   0.050    10      5      0      0         nil)         
  (car             0.08    0.2    0.020     0    100      4      0         nil)
  (convoy          0.20    0.2    0.1       0     10      0      0         nil);;a dummy entry
;;  (supergun        0       0.1    0.01   1000   1000     30     20         super_mortar)

  ; supertank health is determined by upgrade levels defined below
  (stank           0.3     0.2    0.1   10000  10000      0      15        nil)
)

(def_weapon_damage
  ; (sing = hurt single vehicle, mult = do area of effect (using specified radius)
  ; weapon         damage  default  ticks or  speed  range  specific
  ; name           type    damage   radius                  damage  
  ;---------------------------------------------------------
  (b120mm          sing     40       0         1.0   4    (peon_tank 1000))    
  (acid            sing     2        40        0.5   4)   
  (napalm          sing     4        40        0.5   4)   
  (heavy_rocket    mult     500      2         0.4   30)    
  (vortex_missile  mult     750      3         0.4   30)    
  (nuke_missile    mult     1000     4         0.4   30)    

  (chain_gun       sing     3        0         1.0   3)  
  (beam            sing     7        0         0.0   4)    
  (plasma          sing     1        0         0.5   4)
  (bolt            sing     10       0         0.5   5)
  (super_mortar    mult     10000    3         0.0   30)

  (b90mm           sing     20       0         0.5   3)    
  (buster_rocket   mult     1000     1         0.2  18)    
  (guided_missile  sing     10       1         0.3  20)    
  (electric_beam   sing     5        2         0.0   5)    
  (dropped_bomb    mult     100      1.5       0.08  4)
  (trike           mult     1000     2         0.2   5)
  (bomb_truck      mult     50000    3         0.0   0)
  (electric_car    sing     5        2         0.2   1)
  
  (inv_explosion       mult     100      3         0.0   10)
  )


; kill-ratio (points/deaths) when supertank gets upgraded
(setf upgrade_kill_ratio  '(1000 20000))  ; ratios for upgrade-level 2 & 3
            
;; these should be some where else as they are not moving objects
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(defaults      ;     sp    tp     a         c     h      fd     dr
  (verybiggun        0     0.1    0         0   1000     10     6   napalm)
  (turret            0     0.1    0         0    200     25     6   b120mm)
  (popup_turret      0     0.1    0         0    200      0     5   chain_gun)
  (tower_missile     0     0.1    0         0   1000     25     6   guided_missile)
  (tower_electric    0     0.1    0         0   1000     25     6   bolt)
  (supergun          0     0.01   0         0   1200    200    20   super_mortar)
  (super_mortar      0     0.01   0         0   1200    200    10   super_mortar)
  (guided_missile  0.8     0.1    0.030     0    100      0     0   nil)
  (buster_rocket   0.7     0.1    0.020     0    100      0     0   nil)
  (heavy_rocket    0.7     0.1    0.020     0    100      0     0   nil)
  (vortex_missile  0.7     0.1    0.017     0    100      0     0   nil)
  (nuke_missile    0.7     0.1    0.015     0    100      0     0   nil)
  (base_launcher     0     0.2    0         0   1000     50     6   buster_rocket)
  (gunport           0     0.2    0         0    500     10     7   b120mm) ;;+ user defined ones
  (repairer        0.1     0.05   0         0      0      5     3   nil))

(def_stank_weapons
  ;                 max  refuel      fire_delay  iconic name used to load up .tga's
  (guided_missile    40   200            50           "guided") ;; for testing
  (bolt             2000  200             0           "plasma") ;; for testing

  (b120mm            40   200             5           "120mm")
  (acid              40   200             5           "acid")
  (napalm            40   200             5           "napalm")

  (heavy_rocket       2   200            50           "guided")
  (vortex_missile     2   200            50           "vortex")
  (nuke_missile       2   200            50           "nuke")


  (chain_gun         100  200             0           "minigun")
  (beam              150  200             0           "beam")
  (plasma            150  200             0           "plasma")

  (kevlar           500   200             0           "kevlar")
  (reactive         750   200             0           "reactive")
  (titanium        1000   200             0           "titanium")
)


;;defaults for special buildings
;;if not defined here, the building uses the default (100) as maximum health
(defaults
;;                 speed   speed    accel  cost   health  delay  range     w-type
  (garage             0       0       0   50000    4000     0     0        nil)
  (airbase            0       0       0   60000    4000     0     0        nil)
  (mainbasepad        0       0       0   65000    4000     0     0        nil)
  (mainbasefull010698 0       0       0      0     4000     0     0        nil)
  (lawfirm            0       0       0   10000    4000     0     0        nil)
  (bank               0       0       0  100000    4000     0     0        nil)
  )
  
;(special_commands_bitmap_mapping
;  (command-stop "bitmaps/commands/stop.tga")
;  (command-self_destruct "bitmaps/commands/self_destruct.tga")
;  )

