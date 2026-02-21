use yew::prelude::*;

use super::points_input::PointsInput;

use crate::utils::get_css::get_css;

#[function_component(Header)]
pub fn header() -> Html {
    let style_sheet = get_css!("character_sheet/header.css");

    html! {
        <div class={style_sheet}>
            <div class="header">
                <div class="person-info">
                    {render_person_info_textbox("Раса:")}
                    {render_person_info_textbox("Раса:")}
                    {render_person_info_textbox("Игрок:")}
                    
                </div>
                <div class="wounds">
                    <span>{"Раны:"}</span>
                    <div class="wounds-circles">
                        <PointsInput on_change={Callback::from(|_num: usize| ())} />
                    </div>
                </div>
            </div>
        </div>
    }
}

fn render_person_info_textbox(header: &str) -> Html {
    html! {
        <div>
            <span>{header}</span>
            <input type="text" class="text-input" />
        </div>
    }
}
