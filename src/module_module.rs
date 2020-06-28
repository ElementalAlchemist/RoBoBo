use libloading::{Library, Symbol};

pub trait Module {
	fn bot_version(&self) -> u32;
	fn load(&self);
	fn unload(&self);

	// TODO IRC events
}

pub fn spawn_module(name: &str) -> Result<Box<dyn Module>, String> {
	let load_path = format!("modules/lib{}.so", name);
	let module_lib = match Library::new(load_path) {
		Ok(result) => result,
		Err(err) => return Err(format!("Failed to load module {}: {}", name, err))
	};

	let module = unsafe {
		let load_func: Symbol<fn() -> Box<dyn Module>> = match module_lib.get(b"spawn") {
			Ok(func) => func,
			Err(err) => return Err(format!("Module {} doesn't contain a spawn function: {}", name, err))
		};
		load_func()
	};
	Ok(module)
}