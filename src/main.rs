use clap::{App, Arg};
use robobo::CURRENT_BOT_VERSION;

fn main() {
	let option_values = App::new("RoBoBo IRC Bot")
		.version(CURRENT_BOT_VERSION)
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
		.arg(
			Arg::with_name("log_with_stdout")
				.long("log-with-stdout")
				.help("When debugging, enables logging to both the log file and stdout"),
		)
		.get_matches();

	let debug_level = option_values.occurrences_of("debug");
	let debug_level = if debug_level > u32::max_value().into() {
		u32::max_value()
	} else {
		debug_level as u32
	};

	let args = robobo::ProgramArgs {
		config_file_name: option_values.value_of("config").unwrap_or("robobo.conf"),
		debug_level,
		use_log_with_stdout: option_values.is_present("log_with_stdout"),
	};

	robobo::run(&args);
}
