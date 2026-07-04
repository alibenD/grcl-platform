# grcl-runtime-ros2

`grcl-runtime-ros2` is the planned ROS2 adapter and backend projection module for GRCL Platform.

It is responsible for projecting a GRCL runtime view from the ROS2 node graph, backend metadata,
and GRCL-owned state, without exposing `rcl`, `rmw`, or `rclcpp` through public GRCL APIs.
ROS2 does not directly expose a GRCL runtime participant graph; that projection remains the
adapter's responsibility.

No runtime behavior is implemented in this skeleton phase.
