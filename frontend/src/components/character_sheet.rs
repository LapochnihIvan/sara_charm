mod header;
mod points_input;

use yew::prelude::*;

use header::Header;

use crate::utils::get_css::get_css;

#[function_component(CharacterSheet)]
pub fn character_sheet() -> Html {
    let style_sheet = get_css!("character_sheet/character_sheet.css");

    html! {
        <div class={style_sheet}>
            <div class="character-sheet">
                <Header />
            </div>
        </div>
    }
}
