use std::{
    fs::{self, File},
    io::Write,
};

fn main() -> std::io::Result<()> {
    prost_build::compile_protos(
        &["../proto/settings.proto"],
        &["../proto", "../nanopb/generator/proto"],
    )?;

    for img_file in fs::read_dir("img/")? {
        let img_file = img_file?;

        if let Some(file_name) = img_file.path().file_name().and_then(|name| name.to_str()) {
            let file_name = file_name.replace('.', "_");

            let content = fs::read_to_string(img_file.path())?;

            let mut out_file = {
                let out_path = format!(
                    "{}/{file_name}.rs",
                    std::env::var("OUT_DIR").unwrap()
                );

                File::create(out_path)?
            };

            out_file.write_all(
                format!(
                    "pub mod {file_name}{{\n    \
                        pub fn render() -> yew::Html {{\n        \
                            yew::html! {{\n\
                                {content}        \
                            }}\n    \
                        }}\n\
                    }}"
                )
                .as_bytes(),
            )?;
        }
    }
    Ok(())
}
