use yew::{prelude::*, suspense::use_future};
use yew_hooks::prelude::*;

use gloo_net::http::Request;

use web_sys::{HtmlInputElement, js_sys};

use prost::Message;

use crate::img::gear_svg;
use crate::proto::WiFiSettings as WiFiSettingsMsg;
use crate::utils::{alert::alert, get_css::get_css};

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
                    {gear_svg::render()}
                </div>
                if *is_menu_open {
                    <WiFiSettings />
                }
            </div>
        </div>
    }
}

const WIFI_SETTINGS_API: &str = "/api/wifi_settings";

#[function_component(WiFiSettings)]
fn wifi_settings() -> Html {
    let settings = use_future(|| async {
        let settings =
            if let Ok(resp) = Request::get(WIFI_SETTINGS_API).send().await {
                resp.binary().await.unwrap_or_default()
            } else {
                Default::default()
            };

        WiFiSettingsMsg::decode(settings.as_slice()).unwrap_or_default()
    });

    html! {
        <div class="drop-down-menu">
            if let Ok(settings) = settings {
                <WiFiSettingsMenu
                    ssid={settings.ssid.clone()}
                    password={settings.password.clone()}
                ></WiFiSettingsMenu>
            } else {
                <p>{"Загрузка"}</p>
            }
        </div>
    }
}

#[derive(Clone, PartialEq, Properties)]
struct WiFiSettingsMenuProps {
    ssid: String,
    password: String,
}

impl From<&WiFiSettingsMsg> for WiFiSettingsMenuProps {
    fn from(src: &WiFiSettingsMsg) -> Self {
        Self {
            ssid: src.ssid.clone(),
            password: src.password.clone(),
        }
    }
}

#[function_component(WiFiSettingsMenu)]
fn wifi_settings_menu(props: &WiFiSettingsMenuProps) -> Html {
    let ssid_input = use_node_ref();
    let password_input = use_node_ref();

    let send_settings = {
        let ssid_input = ssid_input.clone();
        let password_input = password_input.clone();

        use_async(async move {
            let new_settings = WiFiSettingsMsg {
                ssid: get_input_value(&ssid_input),
                password: get_input_value(&password_input),
            };

            send_wifi_settings(&new_settings)
                .await
                .map(|_| new_settings)
        })
    };

    let settings = {
        let settings = props.clone();
        use_state(move || settings)
    };

    use_effect_with(send_settings.clone(), {
        let settings = settings.clone();
        let ssid_input = ssid_input.clone();
        let password_input = password_input.clone();

        move |send_settings| {
            if send_settings.loading {
                return;
            }

            if let Some(new_settings) = send_settings.data.as_ref() {
                settings.set(new_settings.into());

                alert("Необходимо переподключить к Wi-Fi");
            } else if let Some(e) = send_settings.error.as_ref() {
                alert(&e);

                // Reset text boxes
                set_input_value(&ssid_input, &settings.ssid);
                set_input_value(&password_input, &settings.password);
            }
        }
    });

    html! {
        <>
            <h3 class="settings-header">{"Настройки Wi-Fi"}</h3>

            <p>{"SSID:"}</p>
            <input
                type="text"
                value={settings.ssid.clone()}
                ref={ssid_input}
            />
            <p>{"Пароль:"}</p>
            <input
                type="text"
                value={settings.password.clone()}
                ref={password_input}
            />

            <button
                class="send-btn"
                onclick={Callback::from(move |_| send_settings.run())}
                disabled={send_settings.loading}
            >{"Сохранить"}</button>
        </>
    }
}

async fn send_wifi_settings(settings: &WiFiSettingsMsg) -> Result<(), String> {
    let settings = js_sys::Uint8Array::from(
        settings.encode_to_vec().as_slice(),
    );
    match Request::post(WIFI_SETTINGS_API)
        .body(settings)
        .unwrap()
        .send()
        .await
    {
        Ok(resp) => {
            let status = resp.status();
            match status {
                202 => Ok(()),
                400 => Err(format!(
                    "Ошибка установки настроек Wi-Fi: {}",
                    resp.text().await.unwrap_or_default()
                )),
                status => Err(format!(
                    "Ошибка приёма настроек Wi-Fi на сервере: статус {status}"
                )),
            }
        }
        Err(e) => Err(format!("Ошибка отправки настроек Wi-Fi: {e}")),
    }
}

fn get_input_value(node: &NodeRef) -> String {
    node.cast::<HtmlInputElement>()
        .map(|input| input.value())
        .unwrap_or_default()
}

fn set_input_value(node: &NodeRef, value: &str) {
    if let Some(input) = node.cast::<HtmlInputElement>() {
        input.set_value(value);
    }
}
