[_ISTool]
EnableISX=true

[Files]
Source: {src}\Golgotha.exe; DestDir: {app}; Components: core; Flags: external
Source: {src}\resource.res; DestDir: {app}; Flags: promptifolder external; Components: core
Source: {src}\Manual.doc; DestDir: {app}; Components: manual; Flags: external
Source: Readme.txt; DestDir: {app}; Flags: isreadme; Components: core
Source: {src}\textures\*.*; DestDir: {app}\textures\; Flags: external skipifsourcedoesntexist; Components: textures
Source: {src}\sfx\ambient\wind_and_water_tunnel_mix_22khz_lp.wav; DestDir: {app}\sfx\ambient; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\ambient\eagle1_22khz.wav; DestDir: {app}\sfx\ambient; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\ambient\fast_dripping_water_22khz_lp.wav; DestDir: {app}\sfx\ambient; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\ambient\generic_windy_ambiance_22khz_lp.wav; DestDir: {app}\sfx\ambient; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\ambient\goats_22khz.wav; DestDir: {app}\sfx\ambient; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\ambient\gushing_wind_22khz_lp.wav; DestDir: {app}\sfx\ambient; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\ambient\lava_bubbles_22khz_lp.wav; DestDir: {app}\sfx\ambient; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\ambient\owl1_22khz.wav; DestDir: {app}\sfx\ambient; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\ambient\river_under_bridge_22khz_lp.wav; DestDir: {app}\sfx\ambient; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\ambient\sea_seaguls_shiphorns_22khz_lp.wav; DestDir: {app}\sfx\ambient; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\ambient\snow_level_22khz.wav; DestDir: {app}\sfx\ambient; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\ambient\birds_22khz_lp.wav; DestDir: {app}\sfx\ambient; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\ambient\wolf_howl_22khz.wav; DestDir: {app}\sfx\ambient; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\gentle_wind_1_22khz_lp.wav; DestDir: {app}\sfx; Components: sounds; Flags: external skipifsourcedoesntexist
Source: {src}\sfx\weapon_sounds.wav; DestDir: {app}\sfx; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\wind_22khz.wav; DestDir: {app}\sfx; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\assembled\electric_car.wav; DestDir: {app}\sfx\assembled; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\assembled\engineering_vehicle.wav; DestDir: {app}\sfx\assembled; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\assembled\helicopter.wav; DestDir: {app}\sfx\assembled; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\assembled\jet.wav; DestDir: {app}\sfx\assembled; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\assembled\kamikaze_trike.wav; DestDir: {app}\sfx\assembled; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\assembled\missile_truck.wav; DestDir: {app}\sfx\assembled; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\assembled\money_cue_44khz.wav; DestDir: {app}\sfx\assembled; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\assembled\money_received_22khz.wav; DestDir: {app}\sfx\assembled; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\assembled\money_recieved_two_22khz.wav; DestDir: {app}\sfx\assembled; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\assembled\money_recieved_two_44khz.wav; DestDir: {app}\sfx\assembled; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\assembled\old_vehicle_ready_22khz.wav; DestDir: {app}\sfx\assembled; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\assembled\old_vehicle_ready_44khz.wav; DestDir: {app}\sfx\assembled; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\assembled\peon_tank.wav; DestDir: {app}\sfx\assembled; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\assembled\vehicle_done_22khz (2).wav; DestDir: {app}\sfx\assembled; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\assembled\vehicle_done_22khz.wav; DestDir: {app}\sfx\assembled; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\assembled\vehicle_done_44khz.wav; DestDir: {app}\sfx\assembled; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\assembled\vehicle_ready_22khz.wav; DestDir: {app}\sfx\assembled; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\eng_male_05.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\eng_male_01.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\eng_male_02.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\eng_male_03.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\eng_male_04.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\bank_secured_tak.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\eng_male_06.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\eng_male_07.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\eng_male_09.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\formation_des.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\missle_truck_male_02.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\missle_truck_male_03.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\missle_truck_male_04.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\missle_truck_male_05.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\missle_truck_male_08.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\missle_truck_male_09.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\missle_truck_male_10.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\missle_truck_male_11.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\tank_female_02.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\tank_female_04.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\tank_female_07.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\vssver.scc; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\yes_sir_eng_01.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\yes_sir_tank_male_01.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\computer_voice\yes_sir_tank_male_03.wav; DestDir: {app}\sfx\computer_voice; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\explosion\acid.wav; DestDir: {app}\sfx\explosion; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\explosion\ariplane_bomb_explosion_one_22khz.wav; DestDir: {app}\sfx\explosion; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\explosion\generic.wav; DestDir: {app}\sfx\explosion; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\explosion\ground_vehicle.wav; DestDir: {app}\sfx\explosion; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\explosion\old_generic.wav; DestDir: {app}\sfx\explosion; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\explosion\shockwave.wav; DestDir: {app}\sfx\explosion; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\explosion\super_mortar.wav; DestDir: {app}\sfx\explosion; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\supertank\main1.wav; DestDir: {app}\sfx\fire\supertank; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\supertank\old_auto1_exterrior_lp.wav; DestDir: {app}\sfx\fire\supertank; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\supertank\old_auto1_lp.wav; DestDir: {app}\sfx\fire\supertank; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\supertank\old_main1.wav; DestDir: {app}\sfx\fire\supertank; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\acid2.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\big_bertha_gun_fire_22khz.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\electric_tower_charge_up_22khz.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\electric_tower_firing_three_22khz_lp.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\electric_tower_power_down_22khz.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\fire_one_22khz.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\fire_three.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\fire_two.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\machine_gun_fire_22khz_lp.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\missile_truck.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\old_auto1_lp.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\old_machine_gun_fire_22khz_lp.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\old_peon_tank2.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\peon_tank.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\peon_tank2.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\supergun.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\supertank_acid_main_barrel_interrior_22khz.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\supertank_fires_rocket_22khz.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\fire\supertank_plazma_fireball_22khz.wav; DestDir: {app}\sfx\fire; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\bleep_22khz.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\click_one_22khz.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\click_two_22khz.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\electric_car_charged_lp.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\electro_car_charged_lp.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\health_powerup_three_22khz.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\main_barrel_powerup_22khz.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\main_missile_refuel.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\missile_in_flight_22khz.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\missile_powerup_22khz.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\missile_truck_lower.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\missile_truck_raise.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\old_missile_in_flight_22khz.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\powerup_money_22khz.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\rising_missile_bay.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\rotating_turret.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\supertank_chain_gun_refuel.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\supertank_lvl_one_main_barrel_interrior_44khz.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\supertank_refuel_lp.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\test.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\total_miss_1.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\turbine1_lp.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\vehicle_landing_clank.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\warning_siren_22khz_lp.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\misc\was_a_miss_1.wav; DestDir: {app}\sfx\misc; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\narrative\turret_lost_22khz.wav; DestDir: {app}\sfx\narrative; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\narrative\bank_lost_22khz.wav; DestDir: {app}\sfx\narrative; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\narrative\building_captured_22khz.wav; DestDir: {app}\sfx\narrative; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\narrative\enemy_base_destroyed_22khz.wav; DestDir: {app}\sfx\narrative; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\narrative\mission_failed_22khz.wav; DestDir: {app}\sfx\narrative; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\narrative\money_received_22khz.wav; DestDir: {app}\sfx\narrative; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\narrative\supertank_lost_22khz.wav; DestDir: {app}\sfx\narrative; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\narrative\turret_captured_22khz.wav; DestDir: {app}\sfx\narrative; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\narrative\bank_captured_22khz.wav; DestDir: {app}\sfx\narrative; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\electric_car_ass.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\electric_car_rumble_lp.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\electro_car_charged_lp.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\engineering_vehicle_ass.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\engineering_vehicle_rumble_lp.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\frogs_and_crickets_ambiance_22khz.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\generic_exp.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\good_shot_2.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\ground_vehicle_explodes.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\helicopter_ass.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\helicopter_rumble_lp.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\jet_ass.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\jet_rumble_lp.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\kamikaze_trike_ass.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\large_tank_fires.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\lowering_missile_bay.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\main_barrel_fire_1.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\main_barrel_refuel.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\main_missile_refuel.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\missile_truck_ass.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\missile_truck_rumble_lp.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\missle_truck_firing.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\musk_temple_22khz.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\oil_pump_22khz.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\owl1_22khz.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\peon_tank_ass.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\peon_tank_main_gun_shells.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\peon_tank_rumble_lp.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\rising_missile_bay.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\river_flow_22khz.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\rotating_turret.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\rox_22khz.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\small_tank_fires.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\super_tank_rumble_lp.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\supertank_chaingun_fire.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\supertank_refuel_lp.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\turbine1_lp.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\vehicle_landing_clank.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\water_pump_intake22khz.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\old\water_pump22khz.wav; DestDir: {app}\sfx\old; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\rumble\supertank_lp.wav; DestDir: {app}\sfx\rumble; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\rumble\engineering_vehicle_lp.wav; DestDir: {app}\sfx\rumble; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\rumble\helicopter_lp.wav; DestDir: {app}\sfx\rumble; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\rumble\jet_lp.wav; DestDir: {app}\sfx\rumble; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\rumble\missile_truck_lp.wav; DestDir: {app}\sfx\rumble; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\rumble\old_peon_tank_lp.wav; DestDir: {app}\sfx\rumble; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\rumble\peon_tank_lp.wav; DestDir: {app}\sfx\rumble; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\rumble\electric_car_lp.wav; DestDir: {app}\sfx\rumble; Flags: external skipifsourcedoesntexist; Components: sounds
Source: {src}\sfx\music\Al_Basrah_22khz.WAV; DestDir: {app}\sfx\music; Flags: external skipifsourcedoesntexist; Components: music
Source: {src}\sfx\music\Cairo_Egypt.WAV; DestDir: {app}\sfx\music; Flags: external skipifsourcedoesntexist; Components: music
Source: {src}\sfx\music\Finnland.WAV; DestDir: {app}\sfx\music; Flags: external skipifsourcedoesntexist; Components: music
Source: {src}\sfx\music\Greece.wav; DestDir: {app}\sfx\music; Flags: external skipifsourcedoesntexist; Components: music
Source: {src}\sfx\music\Helsinki_Sweden.wav; DestDir: {app}\sfx\music; Flags: external skipifsourcedoesntexist; Components: music
Source: {src}\sfx\music\Jerusalem_Israel.wav; DestDir: {app}\sfx\music; Flags: external skipifsourcedoesntexist; Components: music
Source: {src}\sfx\music\Munich_Germany.wav; DestDir: {app}\sfx\music; Flags: external skipifsourcedoesntexist; Components: music
Source: {src}\sfx\music\Nakhayb_Iraq.wav; DestDir: {app}\sfx\music; Flags: external skipifsourcedoesntexist; Components: music
Source: {src}\sfx\music\Naples_Italy.wav; DestDir: {app}\sfx\music; Flags: external skipifsourcedoesntexist; Components: music
Source: {src}\sfx\music\Norway_Nephelim_Battle.wav; DestDir: {app}\sfx\music; Flags: external skipifsourcedoesntexist; Components: music
Source: {src}\sfx\music\Rome_Italy_22khz.WAV; DestDir: {app}\sfx\music; Flags: external skipifsourcedoesntexist; Components: music
Source: {src}\sfx\music\Splash_Screen_Opus_22khz.WAV; DestDir: {app}\sfx\music; Flags: external skipifsourcedoesntexist; Components: music
Source: {src}\sfx\music\Turin_Italy.wav; DestDir: {app}\sfx\music; Flags: external skipifsourcedoesntexist; Components: music
Source: {src}\sfx\music\Vienna_Austria.wav; DestDir: {app}\sfx\music; Flags: external skipifsourcedoesntexist; Components: music
Source: {src}\sfx\music\Zurich_Switzerland_22khz.wav; DestDir: {app}\sfx\music; Flags: external skipifsourcedoesntexist; Components: music
Source: {src}\resource\3dcheck.bmp; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\95check.bmp; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\backward.bmp; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\bground.tga; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\cancel.tga; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\closicon.pcx; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\closicon.tga; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\constants.res; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\crosshair.pcx; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\cur00001.cur; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\cur00002.cur; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\delete.bmp; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\downicon.pcx; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\editor.res; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\editor_en.res; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\error.ico; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\fforward.bmp; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\fly_util.res; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\forward.bmp; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\g1.res; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\g1_en.res; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\help.cur; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\helvetica_8.tga; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\i4.res; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\ico00001.ico; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\ico102.ico; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\keymap.res; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\keys.res; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\keys_maxtool.res; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\lefticon.pcx; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\maxtool.ico; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\mfctest.rc2; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\minifwnd.bmp; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\minus.pcx; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\move4way.cur; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\movecurs.pcx; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\nodrop.cur; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\normcurs.pcx; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\normfont.pcx; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\ntcheck.bmp; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\ok.tga; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\pal.pcx; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\play.bmp; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\plus.pcx; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\pointer.cur; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\randnum.txt; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\rewind.bmp; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\rigticon.pcx; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\sarrows.cur; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\splith.cur; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\splitv.cur; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\sqr_curs.pcx; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\sqr2curs.pcx; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\textcurs.pcx; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\trck4way.cur; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\trcknesw.cur; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\trckns.cur; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\trcknwse.cur; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\trckwe.cur; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\upicon.pcx; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\resource\verboten.bmp; DestDir: {app}\resource; Flags: external promptifolder; Components: bitmaps
Source: {src}\scheme\anims.scm; DestDir: {app}\scheme; Flags: external promptifolder; Components: bitmaps
Source: {src}\scheme\balance.scm; DestDir: {app}\scheme; Flags: external promptifolder; Components: bitmaps
Source: {src}\scheme\classes.scm; DestDir: {app}\scheme; Flags: external promptifolder; Components: bitmaps
Source: {src}\scheme\demo_view.scm; DestDir: {app}\scheme; Flags: external promptifolder; Components: bitmaps
Source: {src}\scheme\doctor.scm; DestDir: {app}\scheme; Flags: external promptifolder; Components: bitmaps
Source: {src}\scheme\dunnet.scm; DestDir: {app}\scheme; Flags: external promptifolder; Components: bitmaps
Source: {src}\scheme\flame.scm; DestDir: {app}\scheme; Flags: external promptifolder; Components: bitmaps
Source: {src}\scheme\map_init.scm; DestDir: {app}\scheme; Flags: external promptifolder; Components: bitmaps
Source: {src}\scheme\menu.scm; DestDir: {app}\scheme; Flags: external promptifolder; Components: bitmaps
Source: {src}\scheme\menu_en.scm; DestDir: {app}\scheme; Flags: external promptifolder; Components: bitmaps
Source: {src}\scheme\menu_maxtool.scm; DestDir: {app}\scheme; Flags: external promptifolder; Components: bitmaps
Source: {src}\scheme\models.scm; DestDir: {app}\scheme; Flags: external promptifolder; Components: bitmaps
Source: {src}\scheme\objects.scm; DestDir: {app}\scheme; Flags: external promptifolder; Components: bitmaps
Source: {src}\scheme\preferences.scm; DestDir: {app}\scheme; Flags: external promptifolder; Components: bitmaps
Source: {src}\scheme\start.scm; DestDir: {app}\scheme; Flags: external promptifolder; Components: bitmaps
Source: {src}\scheme\Test.scm; DestDir: {app}\scheme; Flags: external promptifolder; Components: bitmaps
Source: g_decompressed\win32_dx5_tex_cache.dat; DestDir: {app}\g_decompressed; Flags: onlyifdoesntexist; Components: core
Source: {src}\test.level; DestDir: {app}; Flags: external promptifolder; Components: bitmaps
Source: {src}\snow.level; DestDir: {app}; Flags: external promptifolder; Components: bitmaps
Source: {src}\egypt.level; DestDir: {app}; Flags: external promptifolder; Components: bitmaps
Source: {src}\test.scm; DestDir: {app}; Flags: external promptifolder; Components: bitmaps
Source: {src}\snow.scm; DestDir: {app}; Flags: external promptifolder; Components: bitmaps
Source: {src}\egypt.scm; DestDir: {app}; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\cloud.tga; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\comic1.jpg; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\default.tga; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\golg_font_18.bmp; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\golg_font_18.tga; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\golg_logo_big.bmp; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\golgotha_font.tga; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\help.jpg; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\jc1.gif; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\letter-0.bmp; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\letter-1.bmp; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\letter-2.bmp; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\letter-3.bmp; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\letter-4.bmp; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\letter-5.bmp; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\letter-6.bmp; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\letter-7.bmp; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\letter-8.bmp; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\letter-9.bmp; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\letter-comma.bmp; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\letter-dollar.bmp; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\loading.bmp; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\loading_screen.jpg; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\pick1.tga; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\pick2.tga; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\plot.jpg; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\startup.jpg; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\thanks.jpg; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\title_screen.jpg; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\youlose.jpg; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\youwin.jpg; DestDir: {app}\bitmaps; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\build\*.*; DestDir: {app}\bitmaps\build; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\cheats\*.*; DestDir: {app}\bitmaps\cheats; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\commands\*.*; DestDir: {app}\bitmaps\commands; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\cursors\*.*; DestDir: {app}\bitmaps\cursors; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\editor\*.*; DestDir: {app}\bitmaps\editor; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\logos\*.*; DestDir: {app}\bitmaps\logos; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\menu\*.*; DestDir: {app}\bitmaps\menu; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\network\*.*; DestDir: {app}\bitmaps\network; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\old\*.*; DestDir: {app}\bitmaps\old; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\options\*.*; DestDir: {app}\bitmaps\options; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\radar\*.*; DestDir: {app}\bitmaps\radar; Flags: external promptifolder; Components: bitmaps
Source: {src}\bitmaps\stank\*.*; DestDir: {app}\bitmaps\stank; Flags: external promptifolder; Components: bitmaps
Source: {src}\objects\*.*; DestDir: {app}\objects; Flags: external promptifolder; Components: objects
Source: ivcon\Release\ivcon.exe; DestDir: {app}\ivcon; Components: ivcon
Source: {src}\*.h; DestDir: {app}; Flags: external promptifolder; Components: source
Source: {src}\*.cpp; DestDir: {app}; Flags: external promptifolder; Components: source
Source: {src}\Golgotha.dsw; DestDir: {app}; Flags: external promptifolder; Components: source
Source: {src}\Golgotha.dsp; DestDir: {app}; Flags: external promptifolder; Components: source
Source: {src}\Golgotha.sln; DestDir: {app}; Flags: external promptifolder; Components: source
Source: {src}\Golgotha.vcproj; DestDir: {app}; Flags: external promptifolder; Components: source
Source: {src}\map2def\*.*; DestDir: {app}\map2def; Flags: external promptifolder; Components: source
Source: {src}\ivcon\*.*; DestDir: {app}\ivcon; Flags: external promptifolder; Components: source
Source: ivcon\ivcon.elf; DestDir: {app}\ivcon; Components: linux_exec
Source: golgotha.elf; DestDir: {app}; Components: linux_exec
Source: {src}\app\*.*; DestDir: {app}\app; Flags: external promptifolder; Components: source
Source: {src}\area\*.*; DestDir: {app}\area; Flags: external promptifolder; Components: source
Source: {src}\checksum\*.*; DestDir: {app}\checksum; Flags: external promptifolder; Components: source
Source: {src}\compress\*.*; DestDir: {app}\compress; Flags: external promptifolder; Components: source
Source: {src}\device\*.*; DestDir: {app}\device; Flags: external promptifolder; Components: source
Source: {src}\dll\*.*; DestDir: {app}\dll; Flags: external promptifolder; Components: source
Source: {src}\editor\*.*; DestDir: {app}\editor; Flags: external promptifolder; Components: source
Source: {src}\editor\commands\*.*; DestDir: {app}\editor\commands; Flags: external promptifolder; Components: source
Source: {src}\editor\dialogs\*.*; DestDir: {app}\editor\dialogs; Flags: external promptifolder; Components: source
Source: {src}\editor\mode\*.*; DestDir: {app}\editor\mode; Flags: external promptifolder; Components: source
Source: {src}\error\*.*; DestDir: {app}\error; Flags: external promptifolder; Components: source
Source: {src}\file\*.*; DestDir: {app}\file; Flags: external promptifolder; Components: source
Source: {src}\font\*.*; DestDir: {app}\font; Flags: external promptifolder; Components: source
Source: {src}\gui\*.*; DestDir: {app}\gui; Flags: external promptifolder; Components: source
Source: {src}\image\*.*; DestDir: {app}\image; Flags: external promptifolder; Components: source
Source: {src}\init\*.*; DestDir: {app}\init; Flags: external promptifolder; Components: source
Source: {src}\lisp\*.*; DestDir: {app}\lisp; Flags: external promptifolder; Components: source
Source: {src}\loaders\*.*; DestDir: {app}\loaders; Flags: external promptifolder; Components: source
Source: {src}\loaders\jpg\*.*; DestDir: {app}\loaders\jpg; Flags: external promptifolder; Components: source
Source: {src}\loaders\3ds\*.*; DestDir: {app}\loaders\3ds; Flags: external promptifolder; Components: source
Source: {src}\loaders\mp3\*.*; DestDir: {app}\loaders\mp3; Flags: external promptifolder; Components: source
Source: {src}\main\*.*; DestDir: {app}\main; Flags: external promptifolder; Components: source
Source: {src}\memory\*.*; DestDir: {app}\memory; Flags: external promptifolder; Components: source
Source: {src}\math\*.*; DestDir: {app}\math; Flags: external promptifolder; Components: source
Source: {src}\menu\*.*; DestDir: {app}\menu; Flags: external promptifolder; Components: source
Source: {src}\music\*.*; DestDir: {app}\music; Flags: external promptifolder; Components: source
Source: {src}\net\*.*; DestDir: {app}\net; Flags: external promptifolder; Components: source
Source: {src}\network\*.*; DestDir: {app}\network; Flags: external promptifolder; Components: source
Source: {src}\objs\*.*; DestDir: {app}\objs; Flags: external promptifolder; Components: source
Source: {src}\palette\*.*; DestDir: {app}\palette; Flags: external promptifolder; Components: source
Source: {src}\poly\*.*; DestDir: {app}\poly; Flags: external promptifolder; Components: source
Source: {src}\quantize\*.*; DestDir: {app}\quantize; Flags: external promptifolder; Components: source
Source: render\*.*; DestDir: {app}\render; Components: source; Flags: recursesubdirs
Source: video\*.*; DestDir: {app}\video; Components: source; Flags: recursesubdirs
Source: sound\*.*; DestDir: {app}\sound; Components: source; Flags: recursesubdirs
Source: {src}\status\*.*; DestDir: {app}\status; Flags: external promptifolder; Components: source
Source: {src}\string\*.*; DestDir: {app}\string; Flags: external promptifolder; Components: source
Source: {src}\threads\*.*; DestDir: {app}\threads; Flags: external promptifolder; Components: source
Source: {src}\time\*.*; DestDir: {app}\time; Flags: external promptifolder; Components: source
Source: {src}\transport\*.*; DestDir: {app}\transport; Flags: external promptifolder; Components: source
Source: {src}\win95\*.*; DestDir: {app}\win95; Flags: external promptifolder; Components: source
Source: {src}\window\*.*; DestDir: {app}\window; Flags: external promptifolder; Components: source


[Setup]
DontMergeDuplicateFiles=true
AppCopyright=©Patrick Grawehr, crack dot com et al.
AppName=Golgotha
AppVerName=Golgotha V1.0.8.0
DefaultGroupName=Golgotha
DefaultDirName={pf}\Golgotha
DirExistsWarning=yes
AlwaysShowDirOnReadyPage=true
AlwaysShowGroupOnReadyPage=true
BackColor=clRed
BackColor2=clSilver
AppPublisher=Golgotha revival group
UninstallDisplayIcon={app}\Golgotha.exe
ShowTasksTreeLines=true
OutputDir=D:\golgotha
DisableStartupPrompt=true

[Tasks]

[Components]
Name: core; Description: Program Core Components (required); Flags: fixed; Types: fullsource custom typical minimal full
Name: textures; Description: Install the textures; Types: fullsource custom typical full
Name: source; Description: Installs all the golgotha source code (experts only!); Types: fullsource
Name: bitmaps; Description: Core graphics and levels; Types: fullsource custom typical full
Name: sounds; Description: Sound files; Types: fullsource full
Name: music; Description: Music; Types: fullsource full
Name: manual; Description: User Manual; Types: fullsource custom full
Name: objects; Description: All the Model files; Types: fullsource custom typical full
Name: ivcon; Description: IVCON - 3D Format converter utility; Types: custom fullsource
Name: linux_exec; Description: Executables for linux; Types: full

[Types]
Name: full; Description: Full installation
Name: minimal; Description: Minimal installation (not recommended)
Name: typical; Description: Minimal + Textures
Name: custom; Description: Custom installation settings; Flags: iscustom
Name: fullsource; Description: Full installation with source

[Dirs]
Name: {app}\savegame
Name: {app}\g_decompressed; Components: core

[Icons]
Name: {group}\Golgotha; Filename: {app}\Golgotha.exe; WorkingDir: {app}; IconFilename: {app}\Golgotha.exe; IconIndex: 0; Components: core
Name: {group}\Read Readme file; Filename: {app}\Readme.txt; Components: core
Name: {group}\Manual; Filename: {app}\Manual.doc; Flags: createonlyiffileexists
Name: {group}\IVCON; Filename: {app}\ivcon\ivcon.exe; WorkingDir: {app}\ivcon; IconFilename: {app}\ivcon\ivcon.exe; Components: ivcon; Flags: createonlyiffileexists

[Registry]
Root: HKCU; Subkey: """Software\Crack dot Com\Golgotha\1.0"""; ValueType: string; ValueName: installpath; ValueData: {app}; Flags: createvalueifdoesntexist; Components: core
