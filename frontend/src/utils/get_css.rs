macro_rules! include_css {
    ($path:expr) => {{
        include_str!(concat!(env!("CARGO_MANIFEST_DIR"), "/css/", $path))
    }};
}

macro_rules! get_css {
    ($path:expr) => {{
        stylist::Style::new(crate::utils::get_css::include_css!($path)).unwrap()
    }};
}

macro_rules! get_css_global {
    ($path:expr) => {{
        stylist::GlobalStyle::new(crate::utils::get_css::include_css!($path))
            .unwrap()
    }};
}

pub(crate) use {get_css, get_css_global, include_css};
