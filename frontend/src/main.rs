mod components;
mod utils;
mod proto;
mod img;

use components::app::App;

fn main() {
    yew::Renderer::<App>::new().render();
}