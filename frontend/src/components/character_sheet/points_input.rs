use std::array;

use yew::prelude::*;

use web_sys::HtmlInputElement;

use crate::utils::get_css::get_css;

#[derive(PartialEq, Properties)]
pub struct PointsInputProp {
    pub on_change: Callback<usize, ()>,
}

const NUM_POINTS: usize = 5;

#[function_component(PointsInput)]
pub fn points_input(prop: &PointsInputProp) -> Html {
    let style_sheet = get_css!("character_sheet/points_input.css");

    let points = use_ref(|| {
        array::from_fn::<NodeRef, NUM_POINTS, _>(|_| NodeRef::default())
    });

    html! {
        <div class={style_sheet}>
            for (point_num, point) in points.iter().enumerate() {
                <label class="circle-label">
                    <input
                        class="circle-checkbox"
                        type="checkbox"
                        key={point_num}
                        ref={point}
                        onchange={
                            let points = points.clone();
                            let on_change = prop.on_change.clone();

                            Callback::from(move |event: Event| {
                                let input: HtmlInputElement =
                                    event.target_unchecked_into();
                                let num_filled_points = if input.checked() {
                                    for cur_point_num in 0..point_num {
                                        set_input_checked(
                                            &points[cur_point_num],
                                            true
                                        );
                                    }

                                    point_num + 1
                                } else {
                                    for cur_point_num in
                                        point_num + 1..NUM_POINTS {
                                        set_input_checked(
                                            &points[cur_point_num],
                                            false
                                        );
                                    }

                                    point_num
                                };

                                on_change.emit(num_filled_points)
                            })
                        }
                    />
                </label>
            }
        </div>
    }
}

fn set_input_checked(node: &NodeRef, value: bool) {
    if let Some(input) = node.cast::<HtmlInputElement>() {
        input.set_checked(value);
    }
}
