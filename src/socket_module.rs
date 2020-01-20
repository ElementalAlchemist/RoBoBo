use libloading::{Library, Symbol};

pub trait Socket {
	fn connect(&mut self, connect_to: &str) -> Result<(), String>;
	fn read_line(&mut self) -> Result<String, String>;
	fn write_line(&mut self, line: &str) -> Result<(), String>;
	fn close(&mut self);
}

pub fn spawn_socket(name: &str) -> Result<Box<dyn Socket>, String> {
	let load_path = format!("sockets/lib{}.so", name);
	let module_lib = match Library::new(load_path) {
		Ok(result) => result,
		Err(err) => return Err(format!("Failed to load socket module {}: {}", name, err)),
	};

	let socket_module = unsafe {
		let load_func: Symbol<fn() -> Box<dyn Socket>> = match module_lib.get(b"spawn") {
			Ok(func) => func,
			Err(err) => return Err(format!("Module {} doesn't contain a spawn function: {}", name, err)),
		};
		load_func()
	};
	Ok(socket_module)
}
