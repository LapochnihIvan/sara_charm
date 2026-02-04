use yew::prelude::*;
use yew_hooks::prelude::*;

use stylist::{Style, yew::styled_component};

#[styled_component(Settings)]
pub fn settings() -> Html {
    let style_sheet = Style::new(include_str!("../../css/settings.css")).unwrap();
    
    let is_menu_open = use_bool_toggle(false);
    let menu_toggle = {
        let is_open = is_menu_open.clone();
        Callback::from(move |_| is_open.toggle())
    };

    let settings_bar = use_node_ref();
    use_click_away(settings_bar.clone(), {
        let is_open = is_menu_open.clone();
        move |_| {
            is_open.reset();
        }
    });

    html! {
        <div class={style_sheet.get_class_name().to_owned()}>
            <div class="settings-bar" ref={settings_bar}>
                <div class="gear" onclick={menu_toggle}>
                    {include!(concat!(env!("OUT_DIR"), "/gear_svg_nested"))}
                </div>
                if *is_menu_open {
                    <div class="drop-down-menu">
                        // Some inputs here
                        <p>{"SSID:"}</p>
                        <p>{"Пароль:"}</p>
                    </div>
                }
            </div>
        </div>
    }
}
