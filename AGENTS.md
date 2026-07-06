# GRCL Platform AGENTS (Simplified)

## 目标与边界
- 该文件定义 `src/grcl-platform` 仓库内的协作规则。
- 规则为执行约束，不替代代码和文档作为真理来源。

## 必须遵循的源与优先级
- 优先级：仓库文档 > 任务文档 > 会话上下文。
- 新会话应先读取：
  - `docs/status/current-context.md`
  - `README.md`
  - `docs/adr/`
  - `docs/architecture/`
  - `docs/plans/`（如有）
- 系统级与架构性决策必须在上述文档中留痕。

## 启动与路径约定
- 工作区根：`/home/aliben/project/grcl-platform_ws`
- 仓库根：`/home/aliben/project/grcl-platform_ws/src/grcl-platform`
- 在工作区根执行仓库命令时优先使用 `git -C src/grcl-platform ...`。
- 若历史对话与仓库文档冲突，先以仓库文档为准并确认后再修改。

## 任务分流与执行模式
- 每个任务开始前先分类：架构设计、架构迭代、功能开发、缺陷修复、重构、测试/基础设施、文档行为变更、文档编辑、发布治理。
- 影响行为、契约、队列/恢复状态、治理策略的工作不得使用文档编辑 fast-path。
- 当前实现阶段若未过架构方案评审，默认不做运行时/SDK/构建系统代码开发；允许文档、架构分析与规划。
- 实施任务须有 `docs/plans/` 任务计划（依赖、顺序、验证、用户评审点）。
- 实施任务建议遵循 `one task -> one commit`，并在提交后更新一项可恢复状态面（如 `current-context`、计划、任务台账）。

## 架构与实现边界
- `grcl-c` 为运行时核心语义边界（生命周期、对象所有权、SPI、能力交换、语义一致性）。
- `grcl-cpp`、`grcl-py` 与未来 SDK 不得定义独立运行时语义。
- MCU/RTOS 运行时按声明的 profile 实现，不默认覆盖完整 GRCL。
- ROS2 与 GRCL 图语义应明确区分：ROS2 为用户可见节点图，GRCL 为平台参与者图。

## 子代理与审计
- 允许实施任务下发给子代理，但上下文放在 `.local/agentic-runs/<plan-id>/<task-id>/`。
- 实施结果需经过独立审计后，主代理确认通过后再进入下个任务。
- 子代理超时、终止、或未返回结果时不构成完成证据。

## 完成与验收
- 无验证证据不得宣告完成。
- 验证无法本地执行时，必须记录明确阻塞原因（环境、权限、依赖）与可复现条件。

## 构建产物规则
- 默认不在仓库根写入构建产物（`build/`、`install/`、`log/`、虚拟环境、缓存、wheel 等）。
- 默认产物根：`grcl-platform_ws/artifacts`；可被 `GRCL_PLATFORM_ARTIFACT_ROOT` 覆盖。
- `colcon` 必须显式使用产物根下的 `--build-base`、`--install-base`、`--log-base`。
- `cmake` 必须使用 `-B <artifact_root/...>` 做 out-of-source 构建。
