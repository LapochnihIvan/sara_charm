use yew::prelude::*;

use super::settings::Settings;

use crate::utils::get_css::get_css_global;

const SOURCE_CODE_LINK: &str =
    "https://github.com/LapochnihIvan/sara_charm.git";

#[function_component(App)]
pub fn app() -> Html {
    get_css_global!("app.css");

    html! {
        <>
            <Settings />
            <footer>
                <a href={SOURCE_CODE_LINK} target="_blank">{"Исходный код"}</a>
            </footer>
        </>
    }
}
