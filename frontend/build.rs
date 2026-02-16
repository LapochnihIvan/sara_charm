use std::{fs::File, io::Write};

fn main() -> std::io::Result<()> {
    prost_build::compile_protos(
        &["../proto/messages.proto"],
        &["../proto", "../nanopb/generator/proto"],
    )?;

    let path = format!("{}/gear_svg_nested", std::env::var("OUT_DIR").unwrap());
    let mut gear_svg_file = File::create(path)?;
    gear_svg_file.write_all(
        concat!("html_nested!{\n", include_str!("img/gear.svg"), "\n}\n")
            .as_bytes(),
    )
}
