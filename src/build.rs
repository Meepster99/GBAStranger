//! Build script to generate the rust code for the rooms. 

use serde::Deserialize;
use std::{io::{self, Write, BufWriter}, env, path::{Path, PathBuf}, ffi::OsString, fs::{self, File}};
use heck::ToShoutySnakeCase;

/// Structs to represent rooms that are included in the top of the generated rooms.rs. 
const STRUCTS: &'static [u8] = br#"

/// Representation of a room in void stranger. 
struct Room {
    /// The name of the room. Usually matches the file name. 
    name: &'static str,

    /// Width of the room.
    width: u32,

    /// Height of the room.
    height: u32,

    /// Name of gml file called when room is created. 
    creation_code_id: &'static str,

    /// List of layers associated with this room. 
    layers: &'static [Layer],

    /// List of game objects in this room.
    game_objects: &'static [GameObject],
}

/// A layer defined in a [Room]. 
struct Layer {
    /// The layer name.
    layer_name: &'static str, 
    /// Is this layer visible?
    is_visible: bool,
}

/// A game object used in a [Room].
struct GameObject {
    /// The x position of this game object. 
    x: i32, 

    /// The y position of this game object.
    y: i32, 

    /// The definition of this game object. 
    object_definition: &'static str,
}

"#;

// Deserializable version of all structs put here for use with serde.

#[derive(Deserialize)]
struct Room {
    name: String, 
    width: u32,
    height: u32,
    creation_code_id: String,
    layers: Vec<Layer>,
    game_objects: Vec<GameObject>
}

impl Room {
    // /// Format this room as a rust constant. 
    // fn rust_const(&self) -> String {
    //     // Make a comma seperated string of all the layers. 
    //     let layers = self.layers.iter()
    //         .map(Layer::rust_const)
    //         .collect::<Vec<_>>()
    //         .join(",\n\t\t");

    //     // Make a comma seperated list of all the game objects.
    //     let game_objects = self.game_objects.iter()
    //         .map(GameObject::rust_const)
    //         .collect::<Vec<_>>()
    //         .join(",\n\t\t");

    //     format!("Room {{\n\tname: \"{}\", \n\twidth: {}, \n\theight: {}, \n\tcreation_code_id: \"{}\", \n\tlayers: &[\n\t\t{}\n\t], \n\tgame_objects: &[\n\t\t{}\n\t]\n}}", 
    //         self.name, 
    //         self.width,
    //         self.height,
    //         self.creation_code_id,
    //         layers,
    //         game_objects
    //     )
    // }
}

#[derive(Deserialize)]
struct Layer { layer_name: String, is_visible: bool }

impl Layer {
    // /// Format this layer as a rust constant. 
    // fn rust_const(&self) -> String {
    //     format!("Layer {{ layer_name: \"{}\", is_visible: {} }}", self.layer_name, self.is_visible)
    // }
}

#[derive(Deserialize)]
struct GameObject { x: i32, y: i32, object_definition: String }

impl GameObject {
    // /// Format this game object as a rust constant. 
    // fn rust_const(&self) -> String {
    //     format!("GameObject {{ x: {}, y: {}, object_definition: \"{}\" }}", self.x, self.y, self.object_definition)
    // }
}


fn main() -> io::Result<()> {
    // // Re-run if any assets change
    // println!("cargo:rerun-if-changed=assets/");

    // // Generate rust code that includes all rooms from JSON. 
    // let out_dir: OsString = env::var_os("OUT_DIR").unwrap();
    // let out_path: PathBuf = Path::new(&out_dir).join("rooms.rs");
    // let mut out_file: BufWriter<File> = BufWriter::new(File::create(&out_path).expect("created output file"));

    // // Write the struct definitions to the file. 
    // out_file.write_all(STRUCTS)?;
    // eprintln!("wrote struct definitions to {}", out_path.display());

    // // Create a list of all rooms so that we can get a constant array of all of them. 
    // let mut all_rooms = Vec::new();

    // // Walk through all the files in the assets/roomExport
    // for entry in fs::read_dir("misc/ExportData/Room_Export").unwrap() {
    //     // Assume that the whole dir is .json room files. 
    //     // Get the path of the directory entry.
    //     let path: PathBuf = entry?.path();
    //     // Log that we're processing the current file.
    //     eprintln!("Processing {}...", path.display());
    //     // Open the file at that path and deserialize it from json. 
    //     let room: Room = serde_json::from_reader(File::open(&path)?)?;
    //     // The name for the constant should be an all caps version of the room name. 
    //     let const_name = room.name.to_shouty_snake_case();
    //     // Add reference to room name to list of all rooms.
    //     all_rooms.push(format!("&{const_name}"));
    //     // Write a const to the end of the rooms file.
    //     // Use `#[allow(dead_code)]` to spress warnings. 
    //     writeln!(&mut out_file, "#[allow(dead_code)]")?;
    //     // Write room constant.
    //     writeln!(&mut out_file, "const {const_name}: Room = {};\n", room.rust_const())?;
    // }

    // // Add all rooms constant.
    // writeln!(&mut out_file, "#[allow(dead_code)]")?;
    // writeln!(&mut out_file, "const ALL_ROOMS: [&'static Room; {}] = [{}];", all_rooms.len(), all_rooms.join(", "))?;

    // // Flush all changes to the file. It will be closed with the file object is dropped at the end of this function. 
    // out_file.flush()?;

    // If no errors, return OK. 
    return Ok(());
}
