machine_info_ptr bmachine_new();
unit_ptr bmachine_unit_at(machine_info_ptr mi, char *id);
unit_ptr bmachine_create_unit_auto_id(machine_info_ptr mi, char *gearid);
void bmachine_remove_unit(machine_info_ptr mi, unit_ptr u);
