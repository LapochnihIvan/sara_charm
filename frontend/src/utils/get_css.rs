macro_rules! get_css {
    ($path:expr) => {{
        stylist::Style::new(include_str!(concat!(
            env!("CARGO_MANIFEST_DIR"),
            "/css/",
            $path
        )))
        .unwrap()
    }};
}

pub(crate) use get_css;
