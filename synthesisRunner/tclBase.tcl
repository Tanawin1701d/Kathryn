# Create a new project
create_project my_project  PROJECT_NAME -part xc7a200tfbg676-2 -force

# Add sources
add_files { VERILOG_PATH }

# Set top module
set_property top top [current_fileset]
#update_compile_order -fileset current_fileset

# # Add constraints
# add_files {/path/to/constraints.xdc}

# Run lint
#update_compile_order -fileset sources_1
synth_design -top top -part xc7a200tfbg676-2 -lint

# Run synthesis
reset_run synth_1
launch_runs synth_1 -jobs 4
wait_on_run synth_1

# Run implementation
launch_runs impl_1 -jobs 4
wait_on_run impl_1