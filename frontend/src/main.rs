mod components;
mod proto;
mod utils;

use components::app::App;

fn main() {
    yew::Renderer::<App>::new().render();
}