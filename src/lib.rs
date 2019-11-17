mod config;

pub fn run(config_file_name: &str, debug_level: u32) {
	config::read_config(config_file_name); // TODO Use the config
}
