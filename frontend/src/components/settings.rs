use gloo_net::http::Request;
use web_sys::HtmlInputElement;
use yew::{prelude::*, suspense::use_future};
use yew_hooks::prelude::*;

use prost::Message;

use crate::proto::WiFiSettings;
use crate::utils::get_css::get_css;

#[function_component(Settings)]
pub fn settings() -> Html {
    let style_sheet = get_css!("settings.css");

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
                    <DropDownMenu />
                }
            </div>
        </div>
    }
}

#[function_component(DropDownMenu)]
fn drop_down_menu() -> Html {
    let wifi_settings = use_future(|| async {
        if let Ok(resp) = Request::get("/api/wifi_settings").send().await {
            resp.binary().await.unwrap_or_default()
        } else {
            Default::default()
        }
    });

    html! {
        <div class="drop-down-menu">
            if let Ok(wifi_settings) = wifi_settings {
                {render_wifi_settings(&*wifi_settings)}
            } else {
                <p>{"Загрузка..."}</p>
            }
        </div>
    }
}

fn render_wifi_settings(settings: &Vec<u8>) -> Html {
    let settings =
        WiFiSettings::decode(settings.as_slice()).unwrap_or_default();
    html! {
        <>
            <h3 class="settings-header">{"Настройки Wi-Fi"}</h3>

            {render_wifi_settings_option("SSID:", settings.ssid)}
            {render_wifi_settings_option("Пароль:", settings.password)}
        </>
    }
}

fn render_wifi_settings_option(title: &str, start_value: String) -> Html {
    html! {
        <>
            <p>{title}</p>
            <input type="text" value={start_value} />
        </>
    }
}
