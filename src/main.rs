use clap::{App, Arg};

fn main() {
	let option_values = App::new("RoBoBo IRC Bot")
		.version("3.0.0-dev") // TODO Get the version from a more common location
		.about("Customizable bot for IRC")
		.arg(
			Arg::with_name("debug")
				.short("d")
				.long("debug")
				.multiple(true)
				.help("Sets the debug level"),
		)
		.arg(
			Arg::with_name("config")
				.long("config")
				.value_name("FILENAME")
				.takes_value(true)
				.help("Loads the configuration from the specified file instead of the default (robobo.conf)"),
		)
		.get_matches();

	let config_file_name = option_values.value_of("config").unwrap_or("robobo.conf");
	let debug_level = option_values.occurrences_of("debug");
	let debug_level = if debug_level > u32::max_value().into() {
		u32::max_value()
	} else {
		debug_level as u32
	};

	robobo::run(config_file_name, debug_level);
}
