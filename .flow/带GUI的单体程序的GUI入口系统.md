## 1. 想法收集

目标     : 收集用户不成形想法，整理为想法分析文档。用户只描述目标、问题、需求方向，不限定程序类型
前置     : 用户不成形想法文档
解构     : 观察 [收集用户不成形想法 / 整理为想法分析文档。用户只描述目标、问题、需求方向 / 不限定程序类型]
产物     : 想法分析文档
约束     : 用户只描述目标/问题/需求方向，不限定程序类型；静态库说明文档、使用场景描述为可选输入；版本约定：本流程输出的设计文档保存到 `design/v<版本>/gui/`（静态库相关为 `design/v<版本>/gproto/`），版本号由发起方流程（新建与修改程序流程）确定；全流程结束后打同名 git tag；版本约定：本流程输出的设计文档保存到 `design/v<版本>/gui/`（静态库相关为 `design/v<版本>/gproto/`），版本号由发起方流程（新建与修改程序流程）确定；全流程结束后打同名 git tag。
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 想法分析文档 产出且 约束 逐条 满足

------
## 2. 领域分析

目标     : 分析领域模型：业务对象、数据集合、参与者、操作流程
前置     : 想法分析文档
解构     : 观察 [业务对象 / 数据集合 / 参与者 / 操作流程]
产物     : 领域模型文档
约束     : 分析业务对象、数据集合、参与者、操作流程四个要素；领域模型必须建模数据集合（如搜索命中列表、地址列表、进程列表）及其元素操作（查看/复制/编辑/删除/批量添加/批量删除），作为交互模式设计的依据；只有零散对象、未建模集合与元素操作 → 不通过
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 领域模型文档 产出且 约束 逐条 满足

------
## 3. 程序类型判断

目标     : 判断是否适合Qt GUI形式：需要可视化窗口交互？
前置     : 领域模型文档
解构     : 观察 [需要可视化窗口交互]
产物     : 程序类型分析文档
约束     : 适合GUI：需要可视化窗口、鼠标/键盘交互、实时反馈、多控件协作；不适合GUI：纯自动化任务；不适合GUI：仅需简单输入输出；不适合GUI：需要全屏文本界面；判断不通过→跳回领域分析
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 程序类型分析文档 产出且 约束 逐条 满足

------
## 4. 生成GUI需求分析文档

目标     : 生成Qt GUI需求分析文档，含窗口布局描述
前置     : 程序类型分析文档、领域模型文档
解构     : 观察 [生成Qt GUI需求分析文档 / 含窗口布局描述]
产物     : GUI需求分析文档
约束     : 需求需包含窗口布局、用户操作路径（点击流程）、可视化反馈要求
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : GUI需求分析文档 产出且 约束 逐条 满足

------
## 5. 生成GUI功能列表与窗口定义文档

目标     : 拆分GUI功能列表，定义每个窗口
前置     : GUI需求分析文档
解构     : 观察 [拆分GUI功能列表 / 定义每个窗口]
产物     : GUI功能列表与窗口定义文档
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不允许读取输入以外的文件（只允许读取本步骤输入声明的文档）
完成标准 : GUI功能列表与窗口定义文档 产出且 约束 逐条 满足

------
## 6. 交互模式设计

目标     : 从领域模型推导每个数据集合/窗口的人类合理交互：选择模式、操作入口、数据可写性、批量操作、集合形态；布局与交互必须有领域模型依据，禁止 demo 式随意摆放控件
前置     : 领域模型文档、GUI功能列表与窗口定义文档
解构     : 观察 [选择模式、操作入口、数据可写性、批量操作、集合形态；布局与交互必须有领域模型依据 / 禁止 demo 式随意摆放控件]
产物     : 交互模式设计文档
约束     : 对每个数据集合判定性质：只读查询结果（如搜索命中）、可编辑条目集（如地址列表）、单选选择项（如进程列表）；集合性质决定可写性与交互入口；选择模式：操作只作用于单个对象 → 单选；操作可批量（复制/删除/添加多个）→ 必须支持多选（Ctrl/Shift 扩展、全选）与批量执行，禁止只能处理单行；操作入口分级：高频全局操作 → 按钮；作用于单个/选中项的元素级操作 → 双击（主操作）+ 右键上下文菜单（复制/编辑/删除/添加等）；低频管理操作 → 菜单栏；同一操作不重复提供多个入口；可写性：只读集合（查询结果）只允许查看、复制、导出、添加到其他集合，禁止在结果上直接修改数据；可编辑集合才允许增删改，编辑经双击/右键/对话框进入，不把编辑流程堆成面板按钮；集合形态：条目少且固定 → 下拉/单选控件；条目多、可滚动、需多列 → 表格；可编辑多行集合（如地址列表）用表格 + 行编辑，不用单行输入框表达多条目数据；每个窗口先列出数据集合与操作清单，再据此定控件与交互；无领域模型依据的控件/交互 → 不通过
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 交互模式设计文档 产出且 约束 逐条 满足；哪步不符合就跳回哪步（跳回生成GUI需求分析文档或生成GUI功能列表与窗口定义文档）

------
## 7. 状态与反馈信息设计

目标     : 设计所有反馈信息的显示分区与单一来源：持久状态、瞬时反馈、历史日志分类，各自确定唯一显示位置，避免同一信息重复显示、避免状态栏职责混杂
前置     : 交互模式设计文档、GUI需求分析文档
解构     : 观察 [持久状态、瞬时反馈、历史日志分类 / 各自确定唯一显示位置 / 避免同一信息重复显示、避免状态栏职责混杂]
产物     : 状态与反馈信息设计文档
约束     : 反馈信息分三类：持久状态（如连接/就绪/已打开对象，跨操作保持）、瞬时反馈（当前操作结果/进度/错误提示，随操作更新覆盖）、历史日志（操作记录序列，可滚动可追溯）；每类信息各自有唯一显示位置；持久状态：只在一个固定位置显示（状态栏固定区域或专用指示控件，二选一）；同一持久状态不得同时出现在专用控件与状态栏两处；瞬时反馈：集中在状态栏/提示区显示，每次操作覆盖旧反馈，不留存历史；历史日志：用独立日志区/对话框承载（可滚动、可复制、可清空），不得写入状态栏；状态栏不得同时承担日志与瞬时反馈两类职责；错误提示：短暂性错误（输入非法、操作失败）用瞬时反馈或弹窗；需要用户决策的错误用对话框；同一错误只在一个位置呈现；每个窗口列出全部反馈信息点（什么操作、产生什么反馈、显示在哪），无重复无遗漏；同一信息多处显示 → 不通过
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 状态与反馈信息设计文档 产出且 约束 逐条 满足；哪步不符合就跳回哪步（跳回交互模式设计或生成GUI需求分析文档）

------
## 8. 窗口层次结构与导航图

目标     : 分析窗口层次结构与导航图
前置     : GUI功能列表与窗口定义文档、交互模式设计文档、状态与反馈信息设计文档
解构     : 观察 [分析窗口层次结构与导航图]
产物     : 窗口层次结构与导航图
约束     : 使用Qt控件术语（QMainWindow、QWidget、QDockWidget、QTableView等）
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 窗口层次结构与导航图 产出且 约束 逐条 满足

------
## 9. Qt Model/View数据绑定文档

目标     : 分析Qt Model/View数据绑定
前置     : 窗口层次结构与导航图
解构     : 观察 [分析Qt Model/View数据绑定]
产物     : Qt Model/View数据绑定文档
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不允许读取输入以外的文件（只允许读取本步骤输入声明的文档）
完成标准 : Qt Model/View数据绑定文档 产出且 约束 逐条 满足

------
## 10. GUI功能流程图

目标     : 设计GUI功能流程图，含Qt信号/槽触发路径
前置     : GUI功能列表与窗口定义文档
解构     : 观察 [设计GUI功能流程图 / 含Qt信号/槽触发路径]
产物     : GUI功能流程图
约束     : 执行路径需包含：正常路径、错误路径、取消路径（事件驱动的独立路径）
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : GUI功能流程图 产出且 约束 逐条 满足

------
## 11. GUI执行路径文档

目标     : 生成GUI执行路径文档
前置     : GUI功能流程图
解构     : 观察 [生成GUI执行路径文档]
产物     : GUI执行路径文档
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不允许读取输入以外的文件（只允许读取本步骤输入声明的文档）
完成标准 : GUI执行路径文档 产出且 约束 逐条 满足

------
## 12. 用户流程分析

目标     : 分析用户与GUI交互的完整流程：用户操作→View发射信号→VM接收事件触发业务流程（调静态库）→结果反馈→View刷新
前置     : GUI执行路径文档、GUI功能流程图
解构     : 观察 [用户操作→View发射信号→VM接收事件触发业务流程（调静态库）→结果反馈→View刷新]
产物     : 用户流程文档
约束     : 用户流程 = VM接收View事件触发业务流程的过程（业务流程在GUI层的体现）；每个用户操作对应一个用户流程，覆盖正常路径、错误路径、取消路径；用户流程文档作为用户流程测试与E2E测试的依据
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 用户流程文档 产出且 约束 逐条 满足

------
## 13. 技术选型分析

目标     : 分析技术选型
前置     : GUI功能列表与窗口定义文档
解构     : 观察 [分析技术选型]
产物     : 技术选型分析文档
约束     : 默认：C++20/Qt6::Widgets/Qt6::Concurrent/Windows/CMake/vcpkg
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 技术选型分析文档 产出且 约束 逐条 满足

------
## 14. API能力需求分析

目标     : 分析GUI所需的静态库API能力
前置     : GUI执行路径文档、静态库API文档（如有）
解构     : 观察 [分析GUI所需的静态库API能力]
产物     : API能力需求文档
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不允许读取输入以外的文件（只允许读取本步骤输入声明的文档）
完成标准 : API能力需求文档 产出且 约束 逐条 满足

------
## 15. 静态库存在性匹配

目标     : 判断是否存在静态库
前置     : API能力需求文档、已有静态库说明文档（可选）
解构     : 观察 [判断是否存在静态库]
产物     : API能力匹配分析文档
约束     : 无静态库→生成缺失需求并进入"静态库流程调用"，触发静态库流程；有静态库→继续"API充分性审查"
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : API能力匹配分析文档 产出且 约束 逐条 满足

------
## 16. API充分性审查

目标     : 审查静态库公开API是否满足GUI所有功能需求，是否支持QThread并发调用
前置     : API能力需求文档、静态库API文档
解构     : 观察 [审查静态库公开API是否满足GUI所有功能需求 / 是否支持QThread并发调用]
产物     : API能力审查结果
约束     : 满足→继续GUI设计；不足→跳转到静态库流程补充API；ViewModel直接调静态库API，静态库必须是线程安全的；匹配对象只能是静态库公共API，不允许用OS API作为GUI能力的提供方；绝对不允许在GUI中自行实现缺失功能或编造数据绕过编译
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : API能力审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 17. 缺失能力分析

目标     : 记录静态库无法满足的能力清单
前置     : API充分性审查结果
解构     : 观察 [记录静态库无法满足的能力清单]
产物     : API缺失需求文档
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不允许读取输入以外的文件（只允许读取本步骤输入声明的文档）
完成标准 : API缺失需求文档 产出且 约束 逐条 满足

------
## 18. 静态库流程调用

目标     : 进入静态库开发流程的"跨流程需求接收"步骤，从分析阶段开始补充缺失能力
前置     : API缺失需求文档
解构     : 观察 [进入静态库开发流程的"跨流程需求接收"步骤 / 从分析阶段开始补充缺失能力]
产物     : 静态库需求变更文档、静态库流程交接记录
约束     : 静态库需求变更文档必须逐项列出缺失能力、期望公共API、输入输出、异常要求和验收标准；必须生成静态库流程交接记录，并明确跳转目标为静态库流程的"跨流程需求接收"步骤；没有交接记录不得宣称静态库流程已触发；GUI在静态库完成契约返回前停止ViewModel层编码，不得用模拟数据或假接口继续实现；跨流程协作：静态库流程（分析→设计→编码→编译验证）必须接收交接记录，并在完成后输出静态库完成契约；GUI收到完成契约后回退到"静态库完成回退检查"重新匹配
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 静态库需求变更文档、静态库流程交接记录 产出且 约束 逐条 满足

------
## 19. 静态库完成回退检查

目标     : 确认静态库流程确实完成并把可消费的公共接口交还GUI，重新比较GUI能力与静态库公共API；而不是凭"静态库流程已执行"直接继续
前置     : 静态库完成契约（公共头文件、API列表、out/lib/<配置>产物、能力覆盖结果）
解构     : 观察 [确认静态库流程确实完成并把可消费的公共接口交还GUI / 重新比较GUI能力与静态库公共API；而不是凭"静态库流程已执行"直接继续]
产物     : 更新后的API能力匹配分析文档
约束     : 完成契约缺少公共头文件、API列表、Debug/Release库产物或能力覆盖结果→返回静态库流程对应步骤；仍有未覆盖能力→生成或更新API缺失需求文档，跳回静态库流程；全部覆盖→继续GUI设计；不得在匹配完成前实现ViewModel层的静态库调用；逐项列出真实函数、输入输出、错误处理和验证证据；缺少任一项→不得通过匹配
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 更新后的API能力匹配分析文档 产出且 约束 逐条 满足

------
## 20. 模块分析

目标     : 按MVVM 4层架构分析GUI模块划分
前置     : GUI需求分析文档、API能力需求文档
解构     : 观察 [按MVVM 4层架构分析GUI模块划分]
产物     : GUI模块分析文档
约束     : GUI分为4层：View层/Model层/ViewModel层/静态库；View层只画UI，可单独编译；ViewModel持View指针操控View
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : GUI模块分析文档 产出且 约束 逐条 满足

------
## 21. 职责和边界设计

目标     : 明确每层的职责与禁止项
前置     : GUI模块分析文档
解构     : 观察 [明确每层的职责与禁止项]
产物     : GUI职责和边界说明文档
约束     : 每层定义清晰的禁止项；View不依赖ViewModel/静态库，可单独编译；ViewModel通过View*指针操控View并链接事件
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : GUI职责和边界说明文档 产出且 约束 逐条 满足

------
## 22. View层接口设计

目标     : 设计View层接口：控件getter（供VM绑定）+公开UI方法（供VM更新界面）
前置     : 窗口层次结构与导航图
解构     : 观察 [控件getter（供VM绑定）+公开UI方法（供VM更新界面）]
产物     : View层接口文档
约束     : View层头文件只依赖Qt和Model层，不依赖ViewModel/静态库（保证可单独编译）；View 通过公开 getter 暴露控件（如 QPushButton* refreshBtn() const、QLineEdit* filterEdit() const）供 ViewModel 直接 connect 控件信号；View 通过公开 UI 方法供 ViewModel 更新界面（如 setProcessModel(model)、setStatusText(text)、showEditorPage(name)）；View 不声明用户交互信号、不含 private slots（按钮点击等由 ViewModel 直接 connect 控件信号处理）；历史代码的"View 信号 + VM 槽"风格不再采用，新实现一律用控件 getter + VM connect 风格
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : View层接口文档 产出且 约束 逐条 满足

------
## 23. Model层接口设计

目标     : 设计Model层接口：数据结构+QAbstractTableModel子类定义
前置     : Qt Model/View数据绑定文档
解构     : 观察 [数据结构+QAbstractTableModel子类定义]
产物     : Model层接口文档
约束     : Model层只管理数据，不包含业务逻辑
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : Model层接口文档 产出且 约束 逐条 满足

------
## 24. ViewModel接口设计

目标     : 设计ViewModel接口：install()绑定方法+槽函数签名+View*/Model指针成员+资源句柄
前置     : GUI需求分析文档
解构     : 观察 [install()绑定方法+槽函数签名+View*/Model指针成员+资源句柄]
产物     : ViewModel接口文档
约束     : ViewModel继承QObject，含Q_OBJECT宏；ViewModel持有View*指针（绑定引用不拥有），经 install(MainWindow* view) 设置并集中 connect 控件信号；ViewModel持有Model指针与进程句柄等资源句柄，唯一负责其生命周期；VM通过View公开getter读控件状态、公开方法更新UI，不直接访问View内部控件、不使用findChild访问交互控件；main.cpp 流程固定：创建View → 创建ViewModel → viewModel->install(view) → show() → exec()
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : ViewModel接口文档 产出且 约束 逐条 满足

------
## 25. 静态库API调用映射

目标     : 设计ViewModel到静态库API的调用映射
前置     : GUI执行路径文档、静态库API文档
解构     : 观察 [设计ViewModel到静态库API的调用映射]
产物     : 静态库API调用映射文档
约束     : 每个VM业务方法对应一个真实的静态库API调用；无对应API→触发静态库流程补充
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 静态库API调用映射文档 产出且 约束 逐条 满足

------
## 26. 异常处理与错误显示策略

目标     : 设计异常处理与错误显示策略
前置     : GUI职责和边界说明文档
解构     : 观察 [设计异常处理与错误显示策略]
产物     : 异常处理与错误显示策略文档
约束     : 后台线程的异常通过QFuture::onFailed或信号传递到主线程，不得跨线程抛异常
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 异常处理与错误显示策略文档 产出且 约束 逐条 满足

------
## 27. 技术选型

目标     : 根据窗口结构与技术选型分析进行技术选型
前置     : 窗口层次结构与导航图、技术选型分析文档
解构     : 观察 [根据窗口结构与技术选型分析进行技术选型]
产物     : Qt技术选型文档
约束     : 默认技术选型；程序类型：Windows桌面应用程序（exe）；语言：C++20；窗口框架：Qt6 Widgets（通过vcpkg安装）；Qt版本约束：使用Qt6（非Qt5）；Qt模块：Qt6::Widgets、Qt6::Concurrent；UI构建方式：代码构建Widget（优先）/ Qt Designer .ui文件；异步模型：QtConcurrent::run + QFutureWatcher / QThread + 信号；资源管理：.qrc文件（Qt资源系统）；测试框架：gtest + QTest（UI测试）；包管理：vcpkg manifest（项目自己的 vcpkg.json + CMakePresets 接入 CMAKE_TOOLCHAIN_FILE）管理 Qt6、gtest 等三方依赖；链接方式：经 find_library 引用拥有方项目构建输出 ../pmem/out/lib/<配置>/pmem.lib，公共头文件直接来自 ../pmem/include/pmem.h，无 install/ 中间层；链接 Qt6::Widgets + Qt6::Concurrent；静态库命名：依据程序类型命名（进程内存操作类示例为 pmem，其他程序类型的静态库不得命名为 pmem，应根据程序类型另起名称）
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : Qt技术选型文档 产出且 约束 逐条 满足

------
## 28. 代码规范

目标     : 根据统一要求制定Qt GUI代码规范
前置     : 统一要求
解构     : 观察 [根据统一要求制定Qt GUI代码规范]
产物     : Qt GUI代码规范文档
约束     : View层只画UI：View的.cpp中只能包含UI创建、布局、信号发射代码；View可单独编译：View层头文件不得include ViewModel/静态库头文件；ViewModel持View指针：VM通过View公开方法更新UI，不直接操作View内部控件；ViewModel链接事件：VM构造函数中connect(View信号, VM槽)；Model层纯净：Model只管理数据，不包含业务逻辑；所有参与信号/槽的类必须继承QObject并包含Q_OBJECT宏；UI线程绝不阻塞：QApplication::exec()事件循环中不得调用QThread::wait()；后台线程不直接操作UI：后台线程不能直接发射操作QWidget的信号，必须通过QMetaObject::invokeMethod或QObject::connect的Qt::QueuedConnection；图标和样式表使用.qrc文件（Qt资源系统）；GUI生产代码禁止使用平台相关API或其它第三方库，只允许使用流程文件记录的静态库公共API与Qt框架、C/C++标准库
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : Qt GUI代码规范文档 产出且 约束 逐条 满足

------
## 29. 创建文件夹结构

目标     : 创建Qt GUI 4层文件夹结构（GUI项目子目录内，如 gui/ 与 pmem/ 平级、互不嵌套）
前置     : GUI职责和边界说明文档、Qt GUI代码规范文档
解构     : 观察 [创建Qt GUI 4层文件夹结构（GUI项目子目录内，如 gui/ 与 pmem/ 平级 / 互不嵌套）]
产物     : 文件夹
约束     : 创建 src/view/、src/model/、src/viewmodel/；创建 res/（Qt资源目录）；创建 test/unit/、test/integration/、test/target/、test/e2e/（e2e 位于 test/ 内独立目录，不含 CMakeLists.txt，不参与 CMake）；不允许创建文件
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 文件夹 产出且 约束 逐条 满足

------
## 30. 创建resources.qrc空壳

目标     : 创建Qt资源文件骨架
前置     : GUI模块分析文档
解构     : 观察 [创建Qt资源文件骨架]
产物     : res/resources.qrc
约束     : Qt资源文件骨架，不添加实际资源
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : res/resources.qrc 产出且 约束 逐条 满足

------
## 31. 创建main.cpp空壳

目标     : 创建main.cpp空壳
前置     : Qt GUI代码规范文档、GUI模块分析文档
解构     : 观察 [创建main.cpp空壳]
产物     : src/main.cpp
约束     : QApplication + MainWindow创建 + app.exec()，不填充业务内容
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : src/main.cpp 产出且 约束 逐条 满足

------
## 32. 创建View层空壳

目标     : 创建View层头文件和源文件空壳（QMainWindow/QWidget子类）
前置     : GUI模块分析文档、View层接口文档
解构     : 观察 [创建View层头文件和源文件空壳（QMainWindow/QWidget子类）]
产物     : src/view/头文件+源文件
约束     : 空壳QWidget子类，不填充内容；头文件只依赖Qt和Model层（保证可单独编译）
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : src/view/头文件+源文件 产出且 约束 逐条 满足

------
## 33. 创建Model层空壳

目标     : 创建Model层头文件和源文件空壳（数据结构+QAbstractTableModel子类）
前置     : Model层接口文档
解构     : 观察 [创建Model层头文件和源文件空壳（数据结构+QAbstractTableModel子类）]
产物     : src/model/头文件+源文件
约束     : 空壳QAbstractTableModel子类，不填充内容
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : src/model/头文件+源文件 产出且 约束 逐条 满足

------
## 34. 创建ViewModel层空壳

目标     : 创建ViewModel层头文件和源文件空壳（QObject子类）
前置     : ViewModel接口文档
解构     : 观察 [创建ViewModel层头文件和源文件空壳（QObject子类）]
产物     : src/viewmodel/头文件+源文件
约束     : 空壳QObject子类，必须包含Q_OBJECT宏（即使空壳）；不填充信号/槽内容
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : src/viewmodel/头文件+源文件 产出且 约束 逐条 满足

------
## 35. 创建测试文件空壳

目标     : 创建测试文件空壳
前置     : Qt GUI代码规范文档
解构     : 观察 [创建测试文件空壳]
产物     : test/unit/测试文件
约束     : 空壳测试文件，不填充内容
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : test/unit/测试文件 产出且 约束 逐条 满足

------
## 36. 填充View层头文件声明

目标     : 填充View层头文件声明：UI方法+用户交互信号
前置     : View层接口文档
解构     : 观察 [UI方法+用户交互信号]
产物     : View层声明
约束     : 只写UI方法声明和信号声明，不写实现；不include ViewModel/静态库头文件
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : View层声明 产出且 约束 逐条 满足

------
## 37. 填充Model层头文件声明

目标     : 填充Model层头文件声明：列定义+data()/setData()纯虚函数声明
前置     : Model层接口文档
解构     : 观察 [列定义+data()/setData()纯虚函数声明]
产物     : Model层声明
约束     : 只写列定义和纯虚函数声明
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : Model层声明 产出且 约束 逐条 满足

------
## 38. 填充ViewModel头文件声明

目标     : 填充ViewModel头文件声明：Q_OBJECT+槽函数+View*指针成员
前置     : ViewModel接口文档、Qt GUI代码规范文档
解构     : 观察 [Q_OBJECT+槽函数+View*指针成员]
产物     : ViewModel声明
约束     : 只写信号和槽签名+View*指针成员，不写函数体；槽函数必须在public slots:或private slots:区域
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : ViewModel声明 产出且 约束 逐条 满足

------
## 39. 填充静态库API引用

目标     : 在ViewModel层添加静态库API头文件引用
前置     : 静态库API调用映射文档
解构     : 观察 [在ViewModel层添加静态库API头文件引用]
产物     : 静态库API引用
约束     : 只在ViewModel层添加#include引用；不允许引用静态库内部头文件
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 静态库API引用 产出且 约束 逐条 满足

------
## 40. 实现Model层

目标     : 实现Model层（最先实现：data()/setData()实现）
前置     : Model层接口文档
解构     : 观察 [data()/setData()实现）]
产物     : Model层实现
约束     : 最先实现；data()/setData()实现，不包含格式化逻辑
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : Model层实现 产出且 约束 逐条 满足

------
## 41. 实现View层

目标     : 实现View层（其次实现：只画UI）
前置     : View层接口文档、窗口层次结构与导航图
解构     : 观察 [只画UI）]
产物     : View层实现
约束     : 其次实现；Widget创建、Layout布局、setModel、发射用户交互信号；View不调业务层，不include ViewModel/静态库
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : View层实现 产出且 约束 逐条 满足

------
## 42. 实现ViewModel层

目标     : 实现ViewModel层（持View指针，链接事件，调静态库，操控View刷新）
前置     : ViewModel接口文档、静态库API调用映射文档、Qt GUI代码规范文档
解构     : 观察 [实现ViewModel层（持View指针，链接事件，调静态库，操控View刷新）]
产物     : ViewModel实现
约束     : 然后实现；构造函数中connect(View信号, VM槽)；槽函数：校验输入→调静态库API→更新Model→调用View公开方法刷新UI；耗时操作通过QtConcurrent::run启动，结果通过信号回传；每个静态库调用对应一个真实静态库公共API，不做业务，只做类型转换与API调用
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : ViewModel实现 产出且 约束 逐条 满足

------
## 43. 实现Qt资源文件

目标     : 实现Qt资源文件（resources.qrc + 样式表.qss）
前置     : Qt GUI代码规范文档、窗口层次结构与导航图
解构     : 观察 [实现Qt资源文件（resources.qrc + 样式表.qss）]
产物     : Qt资源实现
约束     : 添加图标文件和.qss样式表；路径格式:/prefix/file
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : Qt资源实现 产出且 约束 逐条 满足

------
## 44. 实现main.cpp

目标     : 实现main.cpp（最后实现：组装各层）
前置     : Qt GUI代码规范文档
解构     : 观察 [组装各层）]
产物     : main.cpp实现
约束     : 最后实现；创建Model→创建View（setModel）→创建ViewModel（注入View指针）→show→app.exec()；ViewModel在View之后创建（注入指针），生命周期管理正确；实现顺序（自底向上）：Model→View→ViewModel→资源→main
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : main.cpp实现 产出且 约束 逐条 满足

------
## 45. View纯净检查

目标     : 查View是否只画UI和排列控件？View中没有include ViewModel/静态库头文件？
前置     : GUI职责和边界说明文档
解构     : 观察 [查View是否只画UI和排列控件 / View中没有include ViewModel/静态库头文件]
产物     : 审查结果
约束     : View 只负责创建控件、布局、样式；不包含任何事件处理逻辑；View 不声明 signals、不含 private slots（按钮点击等由 ViewModel 直接 connect 控件信号处理）；View 不知道 ViewModel 存在（无 ViewModel 类型引用、无 include）；View 头文件只依赖 Qt 与 Model 层，可单独编译
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 46. View独立编译检查

目标     : 查View层头文件是否只依赖Qt和Model层，可单独编译？
前置     : GUI职责和边界说明文档
解构     : 观察 [查View层头文件是否只依赖Qt和Model层 / 可单独编译]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 47. ViewModel指针操控检查

目标     : 查ViewModel是否持有View*指针并链接事件？没有直接访问View内部控件？
前置     : GUI职责和边界说明文档
解构     : 观察 [查ViewModel是否持有View*指针并链接事件 / 没有直接访问View内部控件]
产物     : 审查结果
约束     : ViewModel 持有 View* 指针（绑定引用不拥有，m_view = nullptr 初始化），经 main.cpp 注入或 install() 设置；页面拆分时同时持有页面指针（如 ProcessSelectionPage* m_selectionPage），经 View 的 getter 获取；ViewModel 持有 Model 指针（Model 由 VM 持有并负责生命周期）；进程句柄等资源句柄由 ViewModel 唯一持有（构造/打开时获取，析构时释放）；ViewModel 通过 View 公开 getter 读控件状态、公开 UI 方法更新界面；不得 findChild、不得直接访问 View 私有成员
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 48. 后台执行检查

目标     : 查耗时操作是否在后台执行？主线程中没有QThread::wait()？
前置     : GUI职责和边界说明文档
解构     : 观察 [查耗时操作是否在后台执行 / 主线程中没有QThread::wait()]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 49. Model纯净检查

目标     : 查Model是否只管理数据，没有业务逻辑？
前置     : GUI职责和边界说明文档
解构     : 观察 [查Model是否只管理数据 / 没有业务逻辑]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 50. 事件链接检查

目标     : 查控件信号是否都 connect 到 ViewModel 槽？没有遗漏的 Q_OBJECT 宏？
前置     : GUI职责和边界说明文档
解构     : 观察 [查控件信号是否都 connect 到 ViewModel 槽 / 没有遗漏的 Q_OBJECT 宏]
产物     : 审查结果
约束     : 所有控件信号（QPushButton::clicked、QLineEdit::textChanged、QTableView::doubleClicked 等）都在 ViewModel 层的 install()/连接函数中 connect，View 不自行连接业务逻辑；ViewModel 继承 QObject 且含 Q_OBJECT 宏，事件处理以私有槽/私有方法形式集中在 ViewModel；每个用户可达控件都有对应绑定，无遗漏无重复 connect（重复刷新连接需去重）；main.cpp 创建 View 后创建 ViewModel 并调用 install(view) 完成全部绑定，View 侧无需任何连接代码
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 51. View页面拆分检查

目标     : 查页面是否拆分为独立类独立文件（如 ProcessSelectionPage），而非把所有控件混杂在 MainWindow 中
前置     : 窗口层次结构与导航图、GUI职责和边界说明文档
解构     : 观察 [查页面是否拆分为独立类独立文件（如 ProcessSelectionPage） / 而非把所有控件混杂在 MainWindow 中]
产物     : 审查结果
约束     : MainWindow 只做窗口骨架：QStackedWidget 管理页面 + 菜单栏 + 公共控件；每个页面是独立 QWidget 子类（独立 .h/.cpp 文件），页面内控件在页面类自身构建；进程选择页、编辑器页等各页面不得在 MainWindow.cpp 中直接堆叠控件（参考代码仅演示进程选择页独立成文件，编辑器页同样应独立成文件，不得内联在 MainWindow.cpp 的 setupEditorPage 中）；页面切换经 QStackedWidget::setCurrentIndex 由 ViewModel 控制；每个页面类自行暴露 getter 与公开 UI 方法，MainWindow 只持有页面对象指针并提供访问 getter（如 selectionPage()）；页面文件归属 view/ 目录，命名与页面职责一致；无对应页面时不得为凑文件数拆出空页面
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 52. View控件getter暴露检查

目标     : 查 View 是否通过公开 getter 暴露控件、通过公开 UI 方法暴露界面更新入口，供 ViewModel 绑定与操控
前置     : View层接口文档、GUI职责和边界说明文档
解构     : 观察 [查 View 是否通过公开 getter 暴露控件、通过公开 UI 方法暴露界面更新入口 / 供 ViewModel 绑定与操控]
产物     : 审查结果
约束     : View 为每个需绑定的控件提供公开 getter（如 QPushButton* refreshBtn() const、QLineEdit* filterEdit() const），返回裸指针供 ViewModel connect；View 为每个界面更新提供公开 UI 方法（如 setProcessModel(model)、setStatusText(text)、showEditorPage(name)），ViewModel 通过这些方法更新界面；控件指针在 View 构造时创建并唯一归属 View，getter 只读不转让所有权；ViewModel 不得使用 findChild<QWidget*>(objName) 或遍历子控件访问交互控件；纯展示控件（如选中地址标签）允许特例，但优先经 View 公开 getter 暴露（新代码一律用 getter，参考实现的 findChild 取标签即该特例）
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 53. connect绑定归属检查

目标     : 查所有控件信号绑定是否都发生在 ViewModel 层（install() 或连接函数中），View 侧零连接、零事件处理
前置     : main.cpp、ViewModel实现
解构     : 观察 [查所有控件信号绑定是否都发生在 ViewModel 层（install() 或连接函数中） / View 侧零连接、零事件处理]
产物     : 审查结果
约束     : ViewModel 提供 install(MainWindow* view)（或等价连接函数）：保存 view 指针 → 取页面/控件指针 → 集中 connect 全部控件信号 → 执行初始化（如首次枚举）；main.cpp 流程固定为：创建 View → 创建 ViewModel → viewModel->install(view) → show() → exec()；不得在 View 构造函数内做业务 connect；View 不声明 signals、无 private slots、无事件处理代码（符合 View纯净检查）；控件信号全部由 ViewModel 直接 connect；页面切换（showSelectionPage/showEditorPage）由 ViewModel 调用 View 公开方法完成，View 内部无 setCurrentIndex 的业务触发
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 54. 布局与交互合理性审查

目标     : 审查实际布局与交互是否与交互模式设计一致，每个控件与操作入口是否有领域模型依据
前置     : 交互模式设计文档、状态与反馈信息设计文档、窗口层次结构与导航图、GUI需求分析文档
解构     : 观察 [审查实际布局与交互是否与交互模式设计一致 / 每个控件与操作入口是否有领域模型依据]
产物     : 审查结果
约束     : 按钮只承载高频全局操作；元素级操作（编辑/删除/复制单个条目）必须经双击或右键菜单，不得全部堆成按钮；只读集合（如搜索命中列表）不得存在直接修改数据的入口；修改/添加必须指向可编辑集合（如地址列表）；可批量操作的对象必须支持多选（Ctrl/Shift）与批量执行，界面不得只允许单选单行；可编辑多行集合不得用单行输入框 + 按钮表达；应为表格 + 双击/右键编辑；窗口内每个控件都能说明其领域模型依据（对应哪个数据集合、哪种操作、为何用此入口）；无法说明 → 返回交互模式设计；反馈信息分区：状态栏不得同时承担日志与瞬时反馈两类职责；同一持久状态（如连接状态）不得在专用控件与状态栏两处重复显示；历史日志用独立区域，不写入状态栏；与状态与反馈信息设计文档不符 → 返回状态与反馈信息设计
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步（跳回交互模式设计）

------
## 55. main职责检查

目标     : 查main()是否只创建对象+app.exec()，不做业务
前置     : GUI职责和边界说明文档
解构     : 观察 [查main()是否只创建对象+app.exec() / 不做业务]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 56. View职责检查

目标     : 查View是否只做UI绘制，没有业务逻辑
前置     : GUI职责和边界说明文档
解构     : 观察 [查View是否只做UI绘制 / 没有业务逻辑]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 57. Model职责检查

目标     : 查Model是否只做数据管理，没有业务逻辑
前置     : GUI职责和边界说明文档
解构     : 观察 [查Model是否只做数据管理 / 没有业务逻辑]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 58. ViewModel职责检查

目标     : 查ViewModel是否只做事件→业务→UI更新
前置     : GUI职责和边界说明文档
解构     : 观察 [查ViewModel是否只做事件→业务→UI更新]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 59. 静态库API真实性检查

目标     : 查ViewModel调用的静态库API是否真实存在？没有凭空编造？
前置     : GUI职责和边界说明文档、静态库API调用映射文档
解构     : 观察 [查ViewModel调用的静态库API是否真实存在 / 没有凭空编造]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步（触发静态库流程）

------
## 60. 静态库隔离检查

目标     : 查静态库是否没有引用Qt？GUI是否只通过公开API调用静态库？
前置     : GUI职责和边界说明文档
解构     : 观察 [查静态库是否没有引用Qt / GUI是否只通过公开API调用静态库]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 61. 数据审查：真实数据与接口来源检查

目标     : 审查生产代码是否使用模拟数据、硬编码结果、伪造API或模拟成功路径来替代真实的静态库公共API
前置     : GUI源码、静态库API调用映射文档、API能力匹配分析文档、测试设计文档
解构     : 观察 [审查生产代码是否使用模拟数据 / 硬编码结果 / 伪造API或模拟成功路径来替代真实的静态库公共API]
产物     : 审查结果
约束     : 生产代码禁止出现 mock、fake、dummy、sample 等替代业务结果，禁止写死扫描结果、进程结果、读写成功结果或凭空声明不存在的静态库API；每项生产能力必须能追溯到API能力匹配文档中的真实静态库公共API；只有测试专用target和测试代码可以使用隔离的已知测试值，且不得被生产目标链接或调用；发现模拟数据、伪造接口或无法追溯的能力→立即跳回"API充分性审查"阶段，重新比较静态库API与GUI能力；确有API缺失时继续进入缺失能力分析和静态库流程；审查通过→继续重复实现检查
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足

------
## 62. 重复实现检查

目标     : 查ViewModel有没有在静态库已有API的情况下，自己重复实现相同功能（如自行实现进程读取、地址搜索）
前置     : GUI职责和边界说明文档、静态库职责文档
解构     : 观察 [查ViewModel有没有在静态库已有API的情况下 / 自己重复实现相同功能（如自行实现进程读取、地址搜索）]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步（跳回模块设计重新划分职责）

------
## 63. 平台相关与第三方库检查

目标     : 查GUI生产代码是否出现平台相关API或其它第三方库，是否只使用Qt框架、流程文件记录的静态库公共API与C/C++标准库
前置     : GUI职责和边界说明文档、Qt GUI代码规范文档、静态库API调用映射文档、静态库API列表
解构     : 观察 [查GUI生产代码是否出现平台相关API或其它第三方库 / 是否只使用Qt框架、流程文件记录的静态库公共API与C/C++标准库]
产物     : 审查结果
约束     : GUI生产代码禁止直接包含平台相关头文件或调用平台相关API（windows.h、tlhelp32.h、WinAPI等；Qt内部封装不计）；GUI生产代码禁止链接、包含或使用任何Qt以外的第三方库（boost等）；测试代码的第三方依赖按"三方依赖归属检查"执行；GUI生产代码允许的依赖范围仅限：Qt框架（Qt6::Widgets/Qt6::Concurrent）、接口调用映射文档与静态库API列表中记录的静态库公共API、C/C++标准库
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步（跳回实现ViewModel层，改用静态库公共API或C/C++标准库）

------
## 64. 异步执行检查

目标     : 查耗时操作是否都通过QtConcurrent::run异步执行
前置     : GUI职责和边界说明文档
解构     : 观察 [查耗时操作是否都通过QtConcurrent::run异步执行]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 65. 生命周期检查

目标     : 查QObject生命周期是否管理正确？ViewModel不会在View之前销毁？
前置     : GUI职责和边界说明文档
解构     : 观察 [查QObject生命周期是否管理正确 / ViewModel不会在View之前销毁]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 66. 跨线程异常检查

目标     : 查后台线程的异常是否通过信号传递到主线程
前置     : GUI职责和边界说明文档
解构     : 观察 [查后台线程的异常是否通过信号传递到主线程]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 67. 输入校验检查

目标     : 查控件输入（QLineEdit、QComboBox）是否被校验
前置     : GUI职责和边界说明文档
解构     : 观察 [查控件输入（QLineEdit / QComboBox）是否被校验]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 68. 内存管理检查

目标     : 查内存管理：QObject parent-child关系是否设置正确
前置     : GUI职责和边界说明文档
解构     : 观察 [QObject parent-child关系是否设置正确]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 69. 资源路径检查

目标     : 查Qt资源文件（.qrc）中的路径是否与代码中一致
前置     : GUI职责和边界说明文档
解构     : 观察 [查Qt资源文件（.qrc）中的路径是否与代码中一致]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 70. 空指针检查

目标     : 查指针参数方法是否做了nullptr安全保护（ViewModel注入的View*、接受Model*指针的方法需加空指针检查）
前置     : GUI职责和边界说明文档
解构     : 观察 [查指针参数方法是否做了nullptr安全保护（ViewModel注入的View* / 接受Model*指针的方法需加空指针检查）]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 71. 嵌套深度检查

目标     : 查代码有没有超过三重嵌套
前置     : GUI职责和边界说明文档
解构     : 观察 [查代码有没有超过三重嵌套]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 72. 提前返回检查

目标     : 查代码有没有在错误发生后提前返回避免后面代码无意义的执行
前置     : GUI职责和边界说明文档
解构     : 观察 [查代码有没有在错误发生后提前返回避免后面代码无意义的执行]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 73. 头文件越界检查

目标     : 查头文件有没有越界：ViewModel层依赖了View层内部实现？View层包含了ViewModel/静态库头文件？
前置     : GUI职责和边界说明文档
解构     : 观察 [ViewModel层依赖了View层内部实现 / View层包含了ViewModel/静态库头文件]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 74. 单一职责检查

目标     : 查函数有没有违背一个函数只干一件事的最小原则
前置     : GUI职责和边界说明文档
解构     : 观察 [查函数有没有违背一个函数只干一件事的最小原则]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 75. 规范审查

目标     : 查代码有没有符合Qt GUI代码规范
前置     : Qt GUI代码规范文档
解构     : 观察 [查代码有没有符合Qt GUI代码规范]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 76. 独立子目录检查

目标     : 查GUI与静态库是否分别在根目录的不同子目录中（如 gui/ 与 pmem/ 平级、互不嵌套），GUI不得位于静态库目录内部，静态库也不得位于GUI目录内部
前置     : GUI职责和边界说明文档
解构     : 观察 [查GUI与静态库是否分别在根目录的不同子目录中（如 gui/ 与 pmem/ 平级、互不嵌套） / GUI不得位于静态库目录内部 / 静态库也不得位于GUI目录内部]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步（跳回创建文件夹结构）

------
## 77. 项目自包含检查

目标     : 查每个项目子目录（gui/、pmem/）是否各自自包含：拥有自己的 CMakeLists.txt、CMakePresets.json、script/、out/、vcpkg.json（按需）、third_party/（按需）。项目之间不得共享构建配置或产物目录
前置     : GUI职责和边界说明文档
解构     : 观察 [拥有自己的 CMakeLists.txt / CMakePresets.json / script/ / out/ / vcpkg.json（按需） / third_party/（按需）。项目之间不得共享构建配置或产物目录]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步（跳回创建文件夹结构）

------
## 78. 根目录配置隔离检查

目标     : 查根目录是否不存在 CMakeLists.txt、CMakePresets.json —— 构建配置必须分属各项目子目录（gui/CMakeLists.txt、gui/CMakePresets.json、pmem/CMakeLists.txt、pmem/CMakePresets.json）
前置     : GUI职责和边界说明文档
解构     : 观察 [查根目录是否不存在 CMakeLists.txt / CMakePresets.json —— 构建配置必须分属各项目子目录（gui/CMakeLists.txt / gui/CMakePresets.json / pmem/CMakeLists.txt / pmem/CMakePresets.json）]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步（跳回创建文件夹结构）

------
## 79. 构建产物归属检查

目标     : 查构建输出 out/、编译脚本 script/ 是否位于各自项目子目录内；pmem 无其他依赖时不得出现 install/ 或静态库安装脚本；GUI 可执行文件发布需要时才配置 gui/install/；不得出现在根目录，也不得合并到共享的 workspace/ 目录
前置     : GUI职责和边界说明文档
解构     : 观察 [查构建输出 out/、编译脚本 script/ 是否位于各自项目子目录内；pmem 无其他依赖时不得出现 install/ 或静态库安装脚本；GUI 可执行文件发布需要时才配置 gui/install/；不得出现在根目录 / 也不得合并到共享的 workspace/ 目录]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步（跳回创建文件夹结构）

------
## 80. 三方依赖归属检查

目标     : 查 gtest、Qt6 等第三方依赖的归属：已配置 vcpkg（编译环境分析检测到 vcpkg.exe / PATH 中的 vcpkg / VCPKG_ROOT 任一）时必须以 vcpkg manifest 管理（各项目自己的 vcpkg.json + CMakePresets 接入 CMAKE_TOOLCHAIN_FILE）；仅当确认 vcpkg 不可用时才允许按需 vendored 到需要它的每个项目自己的 third_party/ 下，不得共享混放于根目录或 workspace/
前置     : GUI职责和边界说明文档、包管理选型结果
解构     : 观察 [已配置 vcpkg（编译环境分析检测到 vcpkg.exe / PATH 中的 vcpkg / VCPKG_ROOT 任一）时必须以 vcpkg manifest 管理（各项目自己的 vcpkg.json + CMakePresets 接入 CMAKE_TOOLCHAIN_FILE）；仅当确认 vcpkg 不可用时才允许按需 vendored 到需要它的每个项目自己的 third_party/ 下 / 不得共享混放于根目录或 workspace/]
产物     : 审查结果
约束     : 实际采用方案必须与包管理选型结论一致；已配置 vcpkg 却出现 third_party/ 三方源码、或缺失 vcpkg.json / CMAKE_TOOLCHAIN_FILE 接入 → 不通过
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步（跳回创建文件夹结构）

------
## 81. 跨项目依赖检查

目标     : 查跨项目引用是否直接指向拥有方项目的公共头文件和 out/lib/<配置>（如 GUI 经 find_library 引用 ../pmem/out/lib/<配置>），而非复制源码、install/ 中间层或共享/根级目录
前置     : GUI职责和边界说明文档、静态库API调用映射文档
解构     : 观察 [查跨项目引用是否直接指向拥有方项目的公共头文件和 out/lib/<配置>（如 GUI 经 find_library 引用 ../pmem/out/lib/<配置>） / 而非复制源码、install/ 中间层或共享/根级目录]
产物     : 审查结果
约束     : 按 目标 与 前置 执行；不引入 未经 声明 的 约定
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步（跳回创建文件夹结构）

------
## 82. 测试层级检查

目标     : 查测试层级是否清晰：单元测试逐层测内部逻辑（ViewModel槽/Model数据/View方法）、集成测试走通完整MVVM链路（View信号→VM槽→静态库公共API→Model→View刷新，不跳层）、e2e 测真实二进制行为；是否把集成测试当成 e2e 或用 e2e 冒充集成测试
前置     : GUI测试文件、用户流程文档、GUI执行路径文档
解构     : 观察 [单元测试逐层测内部逻辑（ViewModel槽/Model数据/View方法） / 集成测试走通完整MVVM链路（View信号→VM槽→静态库公共API→Model→View刷新，不跳层） / e2e 测真实二进制行为；是否把集成测试当成 e2e 或用 e2e 冒充集成测试]
产物     : 审查结果
约束     : 集成测试必须走通 View发射信号 -> ViewModel槽 -> 静态库公共API -> 更新Model -> View刷新 的完整链路，不能跳过任何一层；e2e 必须真实启动 pmem_gui.exe（必要时配合 pmem_target.exe）注入真实操作并断言真实画面
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 83. e2e独立性检查

目标     : 查 e2e 测试是否为独立体系：脚本位于 test/e2e/ 文件夹（如 gui/test/e2e/），与 unit/integration 同属 test/ 目录，但独立于 CMake —— 目录内不含 CMakeLists.txt、不注册进 CMakeLists、不得与集成测试混杂在 test/integration/
前置     : GUI e2e 测试文件
解构     : 观察 [脚本位于 test/e2e/ 文件夹（如 gui/test/e2e/） / 与 unit/integration 同属 test/ 目录 / 但独立于 CMake —— 目录内不含 CMakeLists.txt、不注册进 CMakeLists、不得与集成测试混杂在 test/integration/]
产物     : 审查结果
约束     : e2e 目录不属于 add_subdirectory 入口，运行方式独立（如 bash gui/test/e2e/verify_all.sh）
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 84. 测试非空检查

目标     : 查单元/集成测试是否为真实用例而非空壳注释：CMakeLists 必须注册可编译运行的 gtest 测试目标，测试源码必须包含实际 TEST/TEST_F 用例
前置     : gui/test/unit、gui/test/integration 的 CMakeLists 与测试源码
解构     : 观察 [CMakeLists 必须注册可编译运行的 gtest 测试目标 / 测试源码必须包含实际 TEST/TEST_F 用例]
产物     : 审查结果
约束     : 单元测试必须真实覆盖 ViewModel 槽/事件链接、Model data()/setData()、View 公开方法，集成测试必须走通完整 MVVM 链路并断言真实业务结果
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足

------
## 85. 头文件包含风格检查

目标     : 查所有 #include 是否只使用 名字/名字/名字 这种无相对路径的形式（如 "model/scanresultmodel.h"、"view/mainwindow.h"、"viewmodel/mainviewmodel.h"、"pmem.h"），禁止出现 ../ 或 ../../ 开头的相对路径。头文件的搜索路径一律通过 CMakeLists.txt 的 target_include_directories() 提供（生产目标包含 src/ 与 ../pmem/include/，测试目标额外包含 test/ 以便引用 unit/test_app.h）
前置     : gui 全部 .h/.cpp 源文件与测试源文件、src/CMakeLists.txt、test 各 CMakeLists.txt
解构     : 观察 [查所有 #include 是否只使用 名字/名字/名字 这种无相对路径的形式（如 "model/scanresultmodel.h"、"view/mainwindow.h"、"viewmodel/mainviewmodel.h"、"pmem.h"） / 禁止出现 ../ 或 ../../ 开头的相对路径。头文件的搜索路径一律通过 CMakeLists.txt 的 target_include_directories() 提供（生产目标包含 src/ 与 ../pmem/include/，测试目标额外包含 test/ 以便引用 unit/test_app.h）]
产物     : 审查结果
约束     : 任何 #include 出现 "../"、"../../" 等相对路径 → 不通过，跳回编码实现阶段重写为 名字/名字 形式并在 CMakeLists.txt 补充 target_include_directories
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 86. 创建CMakeLists.txt

目标     : 创建GUI项目根 CMakeLists.txt（Qt6+静态库链接+AUTOMOC/AUTORCC），并通过 src/ 与 test 下的独立子项目组织生产代码和测试代码
前置     : Qt技术选型文档
解构     : 观察 [创建GUI项目根 CMakeLists.txt（Qt6+静态库链接+AUTOMOC/AUTORCC） / 并通过 src/ 与 test 下的独立子项目组织生产代码和测试代码]
产物     : gui/CMakeLists.txt
约束     : 根 CMakeLists.txt 只负责 project、构建选项、公共构建策略和以下子项目入口，不直接罗列源文件、不直接创建目标：；CMakeLists.txt 不得硬编码构建输出目录（CMAKE_*_OUTPUT_DIRECTORY 归 CMakePresets.json 各预设）；set(CMAKE_AUTOMOC ON)自动处理Q_OBJECT宏；set(CMAKE_AUTORCC ON)自动处理.qrc资源；set(CMAKE_AUTOUIC ON)如果使用.ui文件；add_subdirectory(src)；add_subdirectory(test/target)；add_subdirectory(test/unit)；add_subdirectory(test/integration)；src/CMakeLists.txt 负责生产代码目标（qt_add_executable）；src 下有独立职责的子文件夹时，通过各自的 CMakeLists.txt 和 add_subdirectory() 继续拆分；test/target/CMakeLists.txt 只负责测试目标程序；test/unit/CMakeLists.txt 只负责单元测试；test/integration/CMakeLists.txt 只负责集成测试注册和运行；（可选）View层可单独编译为独立库/目标，用于UI预览与独立测试；静态库通过 find_library 从拥有方项目的构建输出查找（pmem/out/lib/<配置>/），公共头文件直接来自 pmem/include/
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : gui/CMakeLists.txt 产出且 约束 逐条 满足

------
## 87. 创建CMake子项目文件

目标     : 为生产代码、测试目标、单元测试和集成测试分别创建 CMake 子项目文件，并按目录职责拆分构建配置
前置     : Qt技术选型文档
解构     : 观察 [为生产代码、测试目标、单元测试和集成测试分别创建 CMake 子项目文件 / 并按目录职责拆分构建配置]
产物     : gui/src/CMakeLists.txt、gui/test/target/CMakeLists.txt、gui/test/unit/CMakeLists.txt、gui/test/integration/CMakeLists.txt
约束     : gui/src/CMakeLists.txt 只负责生产目标（qt_add_executable，链接 Qt6::Widgets + Qt6::Concurrent + pmem）；gui/test/target/CMakeLists.txt 只负责测试目标程序；gui/test/unit/CMakeLists.txt 只负责单元测试目标和测试注册（gtest + Qt6::Test）；gui/test/integration/CMakeLists.txt 只负责集成测试注册和运行（gtest + Qt6::Test）；项目根 CMakeLists.txt 必须分别通过 add_subdirectory(src)、add_subdirectory(test/target)、add_subdirectory(test/unit)、add_subdirectory(test/integration) 引入这些子项目；每个子项目 CMakeLists.txt 只管理本目录目标、源文件和依赖，不重复定义父目录已经定义的目标，不把生产代码和测试代码混在同一文件
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : gui/src/CMakeLists.txt、gui/test/target/CMakeLists.txt、gui/test/unit/CMakeLists.txt、gui/test/integration/CMakeLists.txt 产出且 约束 逐条 满足

------
## 88. 创建CMakePresets.json

目标     : 创建CMakePresets.json，配置 Debug/Release 预设
前置     : Qt技术选型文档
解构     : 观察 [创建CMakePresets.json / 配置 Debug/Release 预设]
产物     : gui/CMakePresets.json
约束     : 配置Debug/Release，输出到项目内 out/ 目录；输出目录在预设 cacheVariables 中按配置声明（out/bin/<配置>、out/lib/<配置>），不写入 CMakeLists.txt，编译脚本通过 cmake --preset 应用预设；CMakeLists.txt 与 CMakePresets.json 均位于各项目子目录内（根目录不得存在）
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : gui/CMakePresets.json 产出且 约束 逐条 满足

------
## 89. 分析编译环境

目标     : 分析GUI代码所需的编译环境
前置     : Qt技术选型文档、CMakePresets.json
解构     : 观察 [分析GUI代码所需的编译环境]
产物     : 编译环境需求文档
约束     : 明确代码需要的编译工具和版本：cmake版本、编译器（MSVC/mingw）、Qt6（独立安装 6.10.2，通过 QT6_DEBUG_DIR/QT6_RELEASE_DIR 环境变量定位）、静态库链接等
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 编译环境需求文档 产出且 约束 逐条 满足

------
## 90. CMakePresets编译环境检查

目标     : 查 CMakePresets.json 的运行时库策略是否符合约定：Debug 预设必须链接动态运行库（"CMAKE_MSVC_RUNTIME_LIBRARY": "MultiThreadedDebugDLL"，对应 /MDd）；Release 预设必须链接静态运行库（"CMAKE_MSVC_RUNTIME_LIBRARY": "MultiThreaded"，对应 /MT）
前置     : gui/CMakePresets.json
解构     : 观察 [Debug 预设必须链接动态运行库（"CMAKE_MSVC_RUNTIME_LIBRARY": "MultiThreadedDebugDLL"，对应 /MDd）；Release 预设必须链接静态运行库（"CMAKE_MSVC_RUNTIME_LIBRARY": "MultiThreaded"，对应 /MT）]
产物     : 审查结果
约束     : Debug 链接动态库（/MDd）、Release 链接静态库（/MT）为固定约定；任一预设缺失 CMAKE_MSVC_RUNTIME_LIBRARY、或 Release 取值不是 MultiThreaded、或 Debug 取值不是 MultiThreadedDebugDLL → 不通过，跳回配置构建系统步骤；Release(/MT) 若因 vcpkg 以 /MD 构建的 Qt 静态目标（如 Qt6::EntryPoint/qtmain）触发 LNK2038 运行库不匹配，必须用自定义入口点（winmain.cpp 桥接 WinMain→main）等方式绕开，不得把 Release 回退为 MultiThreadedDLL
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足

------
## 91. Qt6独立安装环境配置

目标     : 配置并校验 Qt6 独立安装目录的定位方式：Debug 用共享版 Qt、Release 用静态版 Qt，通过环境变量 + CMakePresets 的 CMAKE_PREFIX_PATH 接入，不经过 vcpkg
前置     : Qt技术选型文档、编译环境需求文档
解构     : 观察 [Debug 用共享版 Qt、Release 用静态版 Qt / 通过环境变量 + CMakePresets 的 CMAKE_PREFIX_PATH 接入 / 不经过 vcpkg]
产物     : QT6_DEBUG_DIR/QT6_RELEASE_DIR 环境变量与 CMakePresets.json 的 CMAKE_PREFIX_PATH
约束     : Debug 使用共享 Qt 6.10.2：QT6_DEBUG_DIR，默认 C:/d/ccpplib/qt6.10.2-windows-x86_64-msvc2022-shared_debug（编译脚本内置兜底值，允许用户以持久化系统/用户环境变量覆盖）；Release 使用静态 Qt 6.10.2：QT6_RELEASE_DIR，默认 C:/d/ccpplib/Qt_6.10.2-static-Release_msvc2022_64/msvc2022_64（编译脚本内置兜底值，允许覆盖）；gui/CMakePresets.json 各预设 cacheVariables 必须声明 CMAKE_PREFIX_PATH：Debug 为 $env{QT6_DEBUG_DIR}、Release 为 $env{QT6_RELEASE_DIR}；缺失或指向 vcpkg_installed → 不通过；编译脚本（build_debug.bat / build_release.bat / build_keep.bat）必须在 cmake --preset 之前设置 Qt 路径兜底块：if "%QT6_DEBUG_DIR%"=="" set QT6_DEBUG_DIR=C:/d/ccpplib/qt6.10.2-windows-x86_64-msvc2022-shared_debug（Debug），if "%QT6_RELEASE_DIR%"=="" set QT6_RELEASE_DIR=C:/d/ccpplib/Qt_6.10.2-static-Release_msvc2022_64/msvc2022_64（Release）；Qt6 不再由 vcpkg manifest 管理：gui/vcpkg.json 不得包含 qtbase，只保留 gtest 等测试库；vcpkg 仅提供 gtest；Qt 前缀与插件为标准布局（<Qt前缀>/plugins），qt.conf 由根 CMakeLists.txt 按 Qt6::Core 目标位置动态生成（shared: bin/../、static: lib/..），不得硬编码 vcpkg_installed 路径
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : QT6_DEBUG_DIR/QT6_RELEASE_DIR 环境变量与 CMakePresets.json 的 CMAKE_PREFIX_PATH 产出且 约束 逐条 满足

------
## 92. 包管理选型

目标     : 按编译环境检测结果确定三方依赖的管理方式：Qt6 走独立安装（QT6_DEBUG_DIR/QT6_RELEASE_DIR + CMAKE_PREFIX_PATH），gtest 走 vcpkg manifest，给出明确结论与依据
前置     : 编译环境需求文档（含 vcpkg 可用性检测结果）
解构     : 观察 [Qt6 走独立安装（QT6_DEBUG_DIR/QT6_RELEASE_DIR + CMAKE_PREFIX_PATH） / gtest 走 vcpkg manifest / 给出明确结论与依据]
产物     : 包管理选型结果
约束     : Qt6 固定使用独立安装目录（共享 Debug / 静态 Release），不纳入 vcpkg manifest，gui/vcpkg.json 不得声明 qtbase；gtest 已配置 vcpkg（检测到 vcpkg.exe / PATH 中的 vcpkg / VCPKG_ROOT 任一）→ 使用 vcpkg manifest 模式（项目 vcpkg.json + CMakePresets 接入 CMAKE_TOOLCHAIN_FILE）管理 gtest，禁止 vendored；仅当确认 vcpkg 完全不可用（检测不到 vcpkg.exe、无 VCPKG_ROOT、且无法安装）时，才允许将三方依赖 vendored 到需要它的项目自己的 third_party/ 下；选型结论必须写明：检测到的 vcpkg 位置/版本、采用 vcpkg manifest 还是 vendored、选择理由；不得含糊、不得默认跳过 vcpkg 直接走 vendored
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 包管理选型结果 产出且 约束 逐条 满足

------
## 93. 创建build_debug.bat

目标     : 编写Windows Debug编译脚本
前置     : Qt技术选型文档
解构     : 观察 [编写Windows Debug编译脚本]
产物     : gui/script/build_debug.bat
约束     : Windows Debug编译脚本；编译准则：WSL→cmd.exe→.bat→MSVC(cl.exe)，纯Linux才用mingw-w64；需设VCPKG_ROOT环境变量，调用vcvars64.bat初始化MSVC环境；需在 cmake --preset 之前设置 Qt 路径兜底块：；rem Qt path -- set your own if using a different location；rem You can also set QT6_DEBUG_DIR as a persistent system/user environment variable；if "%QT6_DEBUG_DIR%"=="" set QT6_DEBUG_DIR=C:/d/ccpplib/qt6.10.2-windows-x86_64-msvc2022-shared_debug
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : gui/script/build_debug.bat 产出且 约束 逐条 满足

------
## 94. 创建build_release.bat

目标     : 编写Windows Release编译脚本
前置     : Qt技术选型文档
解构     : 观察 [编写Windows Release编译脚本]
产物     : gui/script/build_release.bat
约束     : Windows Release编译脚本；编译准则：WSL→cmd.exe→.bat→MSVC(cl.exe)，纯Linux才用mingw-w64；需在 cmake --preset 之前设置 Qt 路径兜底块：；rem Qt path -- set your own if using a different location；rem You can also set QT6_RELEASE_DIR as a persistent system/user environment variable；if "%QT6_RELEASE_DIR%"=="" set QT6_RELEASE_DIR=C:/d/ccpplib/Qt_6.10.2-static-Release_msvc2022_64/msvc2022_64
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : gui/script/build_release.bat 产出且 约束 逐条 满足

------
## 95. 创建build_debug_wsl.sh

目标     : 编写Linux/WSL Debug编译脚本
前置     : Qt技术选型文档
解构     : 观察 [编写Linux/WSL Debug编译脚本]
产物     : gui/script/build_debug_wsl.sh
约束     : Linux/WSL Debug编译脚本；编译准则：WSL→cmd.exe→.bat→MSVC(cl.exe)，纯Linux才用mingw-w64
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : gui/script/build_debug_wsl.sh 产出且 约束 逐条 满足

------
## 96. 创建build_release_wsl.sh

目标     : 编写Linux/WSL Release编译脚本
前置     : Qt技术选型文档
解构     : 观察 [编写Linux/WSL Release编译脚本]
产物     : gui/script/build_release_wsl.sh
约束     : Linux/WSL Release编译脚本；编译准则：WSL→cmd.exe→.bat→MSVC(cl.exe)，纯Linux才用mingw-w64
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : gui/script/build_release_wsl.sh 产出且 约束 逐条 满足

------
## 97. 脚本分类管理

目标     : 新建脚本前先判断生命周期分类，script/ 目录按核心/验证/排查/一次性工具四类存放，避免脚本越积越乱
前置     : script/ 下已有脚本清单
解构     : 观察 [新建脚本前先判断生命周期分类 / script/ 目录按核心/验证/排查/一次性工具四类存放 / 避免脚本越积越乱]
产物     : gui/script/README.md、分类后的脚本目录
约束     : gui/script/ 根目录只保留核心编译/测试脚本：build_debug.bat、build_release.bat、build_keep.bat、build_debug_wsl.sh、build_release_wsl.sh、run_debug_tests.bat、run_release_tests.bat、run_full_integration.bat、run_full_unit.bat；这些是流程文档指名输出或每次构建/回归必用的长期脚本；验证类脚本放 gui/script/verify/（如 check_release_static.bat、dump_dependents.bat、dump_directives.bat）：dumpbin 依赖/指令检查是编译环境审查里 Release 静态链接策略的可验证手段，每次构建后可复用，长期保留；排查类脚本放 gui/script/diagnose/：定位开发期问题时临时创建的工具（如 read_dialog*.ps1 读模态框、check_entrypoint.bat 查链接、rebuild_int.sh、run_build_bg.sh），问题解决并验证后必须删除，不留存；一次性流程工具放 gui/script/tools/：补丁/改写流程文档的脚本（patch_flow*.py），内容并入文档后删除或归档；新建脚本必须先归入四类之一并同步 README.md；无法归类的临时脚本一律视为排查类，用完即删；脚本内部引用同级或上级脚本优先用绝对路径（或相对 gui/ 的路径），目录移动不破坏引用
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : gui/script/README.md、分类后的脚本目录 产出且 约束 逐条 满足；哪步不符合就跳回哪步（跳回编写编译脚本）

------
## 98. 检测编译环境

目标     : 检测编译环境是否就绪
前置     : 编译环境需求文档
解构     : 观察 [检测编译环境是否就绪]
产物     : 编译环境检测报告
约束     : 检查cmake --version、编译器（cl.exe/g++）、vcpkg、gtest、Qt6等工具是否存在，记录检测结果；检查 QT6_DEBUG_DIR 目录存在且含 bin/Qt6Cored.dll 等共享库（Debug）、QT6_RELEASE_DIR 目录存在且含 lib/Qt6Core.lib 等静态库（Release），不存在→提示用户安装对应 Qt 6.10.2 或修正环境变量；如果工具都存在但编译失败→修复编译脚本和构建配置，不是修改编译环境
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 编译环境检测报告 产出且 约束 逐条 满足

------
## 99. 静态库交付检查

目标     : 确认 pmem 是无其他依赖的静态库，不创建 pmem/install/ 目录，不编写或执行 pmem 静态库安装脚本
前置     : 静态库编译产物（pmem/out/lib/<配置>/）、pmem/include/pmem.h
解构     : 观察 [确认 pmem 是无其他依赖的静态库 / 不创建 pmem/install/ 目录 / 不编写或执行 pmem 静态库安装脚本]
产物     : GUI 直接引用 pmem/out/lib/<配置>/pmem.lib 和 pmem/include/pmem.h
约束     : GUI 必须按 Debug/Release 配置直接查找 pmem/out/lib/<配置>/；pmem 公共头文件直接来自 pmem/include/；如果 pmem/out/lib/<配置>/ 下没有编译产物→跳转到静态库编译步骤
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : GUI 直接引用 pmem/out/lib/<配置>/pmem.lib 和 pmem/include/pmem.h 产出且 约束 逐条 满足

------
## 100. CMake子项目职责和单一原则检查

目标     : 审查 CMakeLists.txt 是否把 src/、test/target、test/unit、test/integration 作为独立子项目，以保持配置简洁和单一职责原则
前置     : 项目根 CMakeLists.txt、src/ 与 test/ 下的 CMakeLists.txt
解构     : 观察 [审查 CMakeLists.txt 是否把 src/、test/target、test/unit、test/integration 作为独立子项目 / 以保持配置简洁和单一职责原则]
产物     : 审查结果
约束     : 根 CMakeLists.txt 只能保留项目级配置、公共选项、输出策略和以下四个入口，不得承担生产源文件列表、测试源文件列表或具体目标的细节：；add_subdirectory(src)；add_subdirectory(test/target)；add_subdirectory(test/unit)；add_subdirectory(test/integration)；src/CMakeLists.txt 只负责生产代码；test/target/CMakeLists.txt 只负责测试目标；test/unit/CMakeLists.txt 只负责单元测试；test/integration/CMakeLists.txt 只负责集成测试注册和运行；每个 CMakeLists.txt 只能管理本目录职责范围内的目标，不能跨目录重复定义目标、重复罗列源文件或混合生产代码与测试代码；目录没有独立构建职责时可以不创建 CMakeLists.txt，但不得为了形式拆分出空的或重复的子项目
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步（跳回创建CMakeLists.txt或创建对应子项目文件）

------
## 101. 编译环境审查

目标     : 审查构建配置与编译脚本，确认生成器/工具链组合只允许 MSVC + Ninja，不允许出现其他组合
前置     : CMakeLists.txt、CMakePresets.json、script/ 下的编译脚本
解构     : 观察 [审查构建配置与编译脚本 / 确认生成器/工具链组合只允许 MSVC + Ninja / 不允许出现其他组合]
产物     : 审查结果
约束     : CMakePresets.json 的 generator 只允许 Ninja；生成器只允许 Ninja：脚本使用 cmake --preset 时由预设 generator 决定，使用 -G 参数时同样只允许 Ninja，不允许 Visual Studio/MSBuild、Makefiles、MinGW Makefiles 等其它生成器；工具链只允许 MSVC（cl.exe，经 vcvars64 环境提供）；不允许 mingw/gcc/clang 等其它编译器；审查脚本是否在 vcpkg/CMake 操作前设置 `set GIT_SSL_NO_VERIFY=1`，用于规避项目记录的 vcpkg SSL 下载问题。；CMakeLists.txt 不得强制其它编译器/生成器分支（如非 MSVC 的编译选项/宏定义）；构建输出目录必须按配置声明在 CMakePresets.json 各预设的 cacheVariables 中（CMAKE_RUNTIME_OUTPUT_DIRECTORY/CMAKE_LIBRARY_OUTPUT_DIRECTORY/CMAKE_ARCHIVE_OUTPUT_DIRECTORY → out/bin/<配置>、out/lib/<配置>）；CMakeLists.txt 不得硬编码 set(CMAKE_*_OUTPUT_DIRECTORY ...)，脚本也不得在 cmake 调用时重复输出目录参数；运行时链接策略必须显式声明在 CMakePresets.json 各预设的 cacheVariables 中（CMAKE_MSVC_RUNTIME_LIBRARY），具体策略（如 Debug 动态 /MDd、Release 静态 /MT）由项目技术选型决定并记录，不得依赖编译器默认；VCPKG_TARGET_TRIPLET 必须与声明的运行时匹配（Debug=x64-windows、Release=x64-windows-static），防止 LNK2038 运行库不匹配；Release 的 VCPKG_TARGET_TRIPLET 必须用静态 triplet x64-windows-static（而非 x64-windows）：vcpkg 按 /MD 构建的 gtest.dll 与 /MT 测试 exe 属 CRT 混链（/MT exe 依赖 /MD DLL），即便用例通过也属运行库一致性违规，必须用 x64-windows-static 让 gtest 以静态 /MT 链接。注意：混链本身不是退出崩溃的根因（见下方 QApplication 泄漏条目），静态 triplet 是 CRT 一致性要求；CMAKE_PREFIX_PATH 必须声明在 CMakePresets.json 各预设的 cacheVariables 中：Debug=$env{QT6_DEBUG_DIR}、Release=$env{QT6_RELEASE_DIR}；脚本必须在 cmake --preset 前设置 QT6_DEBUG_DIR/QT6_RELEASE_DIR 兜底值；指向 vcpkg_installed 或缺失 → 不通过；运行时链接策略必须可验证：Release 产物经 `dumpbin /dependents` 应无 VCRUNTIME/MSVCP DLL 依赖（静态）或如预期依赖（动态），Debug 依赖 VCRUNTIME*D.dll/MSVCP*D.dll；静态库经 `dumpbin /directives` 抽查 /DEFAULTLIB 与策略一致
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步（跳回创建CMakeLists.txt/创建CMakePresets.json/编写编译脚本）

------
## 102. vcpkg manifest 配置检查

目标     : 查 vcpkg 三方依赖管理配置是否完整：vcpkg.json 的 dependencies 是否只包含测试所需三方库（如 gtest，Qt6 已由独立安装目录管理，不得出现 qtbase）；是否配置 builtin-baseline；基线是否经官方命令 `vcpkg.exe x-update-baseline --add-initial-baseline` 生成（而非手工编造提交号），项目内各 vcpkg.json（如 gui/ 与 pmem/）的基线是否一致
前置     : 项目 vcpkg.json、vcpkg 工具
解构     : 观察 [vcpkg.json 的 dependencies 是否只包含测试所需三方库（如 gtest，Qt6 已由独立安装目录管理，不得出现 qtbase）；是否配置 builtin-baseline；基线是否经官方命令 `vcpkg.exe x-update-baseline --add-initial-baseline` 生成（而非手工编造提交号） / 项目内各 vcpkg.json（如 gui/ 与 pmem/）的基线是否一致]
产物     : 审查结果
约束     : vcpkg.json 存在且 dependencies 包含 gtest 等测试三方库；不得包含 qtbase（Qt6 由 QT6_DEBUG_DIR/QT6_RELEASE_DIR 独立安装管理）；vcpkg.json 配置了 builtin-baseline 字段（如 "builtin-baseline": "<commit>"）；基线统一使用官方命令在 vcpkg.json 所在项目目录生成：vcpkg.exe x-update-baseline --add-initial-baseline（需 GIT_SSL_NO_VERIFY=1）；项目内各 vcpkg.json 的 builtin-baseline 必须一致
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 审查结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步（跳回创建CMakePresets.json/编写编译脚本）

------
## 103. 执行编译

目标     : 执行编译验证
前置     : GUI所有源码、CMakeLists.txt
解构     : 观察 [执行编译验证]
产物     : pmem_gui.exe
约束     : 必须使用gui/script/下的编译脚本（build_debug.bat / build_debug_wsl.sh）；禁止直接使用裸cmake命令——编译脚本已包含环境初始化和清理步骤；编译失败时先清理旧产物再重试；确认Qt6库通过 CMakePresets 的 CMAKE_PREFIX_PATH（QT6_DEBUG_DIR/QT6_RELEASE_DIR）正确链接；静态 Qt（Release）下 add_executable 不会像 qt_add_executable 那样自动导入平台插件，必须按 Qt6::Core LOCATION 判定静态后显式调用 qt_import_plugins（生产 exe 导入 Qt6::QWindowsIntegrationPlugin，测试 exe 导入 Qt6::QOffscreenIntegrationPlugin）并 qt_finalize_executable；共享 Qt（Debug）插件由 qt.conf 运行期加载，无需处理；gtest 测试的 QApplication（test_app.h 的 TestAppInstance）必须刻意泄漏（new 后不 delete）：静态 Qt（Release）下若用 static unique_ptr，进程退出时 QApplication 析构与静态 Qt 插件/线程池的静态析构顺序冲突，全部用例通过后仍会 Access Violation（退出码 -1073741819，实测在 QtConcurrent 线程池 worker 线程中触发；换静态 gtest triplet 后依然崩溃，泄漏 QApplication 后退出码才为 0，确认根因即此）；测试进程退出即回收内存，泄漏无害；pmem/out/lib/<配置> 下未找到库文件→跳转静态库编译步骤
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : pmem_gui.exe 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 104. 配置测试框架

目标     : 集成gtest + Qt Test框架
前置     : Qt GUI代码规范文档
解构     : 观察 [集成gtest + Qt Test框架]
产物     : test/的CMakeLists.txt
约束     : 集成gtest和Qt6::Test（提供QSignalSpy / QTRY_VERIFY_WITH_TIMEOUT）；不允许自己编写入口函数（使用gtest main）；已配置 Windows vcpkg 时使用 vcpkg manifest（项目 vcpkg.json + CMAKE_TOOLCHAIN_FILE）；未配置时按三方依赖归属检查 vendored 到项目 third_party/
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : test/的CMakeLists.txt 产出且 约束 逐条 满足

------
## 105. 准备测试目标程序

目标     : 准备测试目标程序（pmem_target.exe），供跨进程场景（集成测试/E2E）使用
前置     : pmem/test/target/main.cpp（或编译好的二进制）
解构     : 观察 [准备测试目标程序（pmem_target.exe） / 供跨进程场景（集成测试/E2E）使用]
产物     : gui/test/target/main.cpp + CMakeLists.txt（或二进制文件）
约束     : 优先复制源码到gui/test/target/由GUI自己的CMake编译；如果直接使用二进制，从pmem/out/bin/Debug/pmem_target.exe复制；target不链接pmem库，关闭ASLR；target在已知地址放已知值（volatile全局变量），循环sleep等待GUI连接检测；target启动时输出美观banner和变量名/地址/值对照表，PID行格式固定为`PID: <number>`，方便人工观察和AI解析
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : gui/test/target/main.cpp + CMakeLists.txt（或二进制文件） 产出且 约束 逐条 满足

------
## 106. ViewModel单元测试

目标     : 编写ViewModel单元测试：事件链接、槽函数、静态库调用链
前置     : ViewModel接口文档
解构     : 观察 [事件链接 / 槽函数 / 静态库调用链]
产物     : test/unit/test_viewmodel.cpp
约束     : 使用QSignalSpy监控信号发射；使用QTest::mouseClick模拟按钮点击→验证VM槽被调用；可注入mock静态库（或真实静态库）验证调用链；避开QMessageBox::warning阻塞路径
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : test/unit/test_viewmodel.cpp 产出且 约束 逐条 满足

------
## 107. Model单元测试

目标     : 编写Model单元测试：data()返回正确值、setData()更新数据
前置     : Model层接口文档
解构     : 观察 [data()返回正确值 / setData()更新数据]
产物     : test/unit/test_model.cpp
约束     : 测试QAbstractTableModel子类的data()/setData()
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : test/unit/test_model.cpp 产出且 约束 逐条 满足

------
## 108. View层单元测试

目标     : 编写View层单元测试：UI方法+信号发射，验证可单独编译
前置     : View层接口文档
解构     : 观察 [UI方法+信号发射 / 验证可单独编译]
产物     : test/unit/test_view.cpp
约束     : View可单独编译为测试目标，不链接ViewModel/静态库；测试View公开方法（setProcessModel、setStatus、showError等）；使用QSignalSpy监听用户交互信号发射
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : test/unit/test_view.cpp 产出且 约束 逐条 满足

------
## 109. 错误弹窗测试

目标     : 编写错误弹窗测试：QTimer::singleShot关闭QMessageBox::warning
前置     : 异常处理与错误显示策略文档
解构     : 观察 [QTimer::singleShot关闭QMessageBox::warning]
产物     : test/unit/测试文件
约束     : 使用QTimer::singleShot(200ms)在对话框exec()嵌套事件循环中关闭弹窗；QApplication::topLevelWidgets() + qobject_cast<QMessageBox*>查找弹窗并验证内容；同时覆盖ViewModel触发和View层直接调用两种路径
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : test/unit/测试文件 产出且 约束 逐条 满足

------
## 110. 执行单元测试

目标     : 执行单元测试
前置     : test/unit/测试文件
解构     : 观察 [执行单元测试]
产物     : 单元测试结果
约束     : 全部通过→继续到集成测试；失败→修复后重试（跳回编写单元测试）
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 单元测试结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 111. 编写集成测试

目标     : 编写MVVM完整流程集成测试（QSignalSpy监控）
前置     : GUI执行路径文档
解构     : 观察 [编写MVVM完整流程集成测试（QSignalSpy监控）]
产物     : test/integration/test_mvvm_flow.cpp
约束     : 测试完整流程：View发射信号→VM槽→调静态库→更新Model→View刷新；使用QSignalSpy监控VM信号发射；使用QTRY_VERIFY_WITH_TIMEOUT异步等待
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : test/integration/test_mvvm_flow.cpp 产出且 约束 逐条 满足

------
## 112. 执行集成测试

目标     : 执行集成测试
前置     : test/integration/测试文件
解构     : 观察 [执行集成测试]
产物     : 集成测试结果
约束     : 涉及真实抓包的用例需管理员令牌（Windows `RunAs` 提权；WSL `sudo`/`setcap`/`wsl -u root`），否则按环境跳过，详见 design/v<版本>/gui/33_admin_capture_guide.md；全部通过→继续编写用户流程测试；失败→修复后重试
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 集成测试结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 113. 编写用户流程测试

目标     : 编写用户流程测试：模拟用户操作（触发View信号）→VM接收事件→触发业务流程（调静态库）→更新Model→View刷新
前置     : 用户流程文档、GUI执行路径文档
解构     : 观察 [模拟用户操作（触发View信号）→VM接收事件→触发业务流程（调静态库）→更新Model→View刷新]
产物     : test/integration/test_user_flow.cpp
约束     : 用户流程测试测试完整用户流程：用户操作到结果反馈的整个链路；VM接收View事件触发业务流程即GUI层的业务流程测试（对应静态库集成测试用真实数据测试的业务流程）；每个用户流程（场景）一个测试用例，使用真实静态库数据验证业务流程真实完成；覆盖正常流程与异常流程（错误输入、失败路径）；与集成测试的区别：集成测试验证MVVM信号/槽机制与数据流正确性；用户流程测试以用户视角验证完整业务流程真实完成（断言真实业务结果）；E2E测试也会覆盖此业务流程（真实启动程序注入用户操作）
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : test/integration/test_user_flow.cpp 产出且 约束 逐条 满足

------
## 114. 执行用户流程测试

目标     : 执行用户流程测试
前置     : test/integration/test_user_flow.cpp
解构     : 观察 [执行用户流程测试]
产物     : 用户流程测试结果
约束     : 涉及真实抓包的用例需管理员令牌（Windows `RunAs` 提权；WSL `sudo`/`setcap`/`wsl -u root`），否则按环境跳过，详见 design/v<版本>/gui/33_admin_capture_guide.md；全部通过→继续到E2E测试；失败→修复后重试
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 用户流程测试结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 115. E2E测试准备

目标     : 准备E2E测试环境：确保test/e2e/脚本目录存在（文件夹结构步骤已建），创建out/verify/产物目录，清理旧进程与旧产物
前置     : 编译产物pmem_gui.exe、GUI执行路径文档
解构     : 观察 [确保test/e2e/脚本目录存在（文件夹结构步骤已建） / 创建out/verify/产物目录 / 清理旧进程与旧产物]
产物     : E2E测试环境
约束     : 启动前先清理旧实例（taskkill/Stop-Process）并删除旧的截图与日志；确保test/e2e/目录存在（用例/驱动/断言/执行脚本），创建out/verify/目录（截图/日志产物）；启动程序时按需禁用垂直同步/动画等影响画面可复现性的设置（如 RENDERER_VSYNC=0 一类模式），保证画面稳定可复现
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : E2E测试环境 产出且 约束 逐条 满足

------
## 116. 编写E2E测试用例

目标     : 先把用户流程与功能逐条翻译成 E2E 测试用例（场景/前置/操作步骤/预期结果），再据此写驱动与断言脚本；用例是功能需求到可执行验证的桥梁，防止"有功能无入口、有入口无验证"
前置     : 用户流程文档、GUI执行路径文档、交互模式设计文档、GUI功能列表与窗口定义文档
解构     : 观察 [先把用户流程与功能逐条翻译成 E2E 测试用例（场景/前置/操作步骤/预期结果） / 再据此写驱动与断言脚本；用例是功能需求到可执行验证的桥梁 / 防止"有功能无入口、有入口无验证"]
产物     : test/e2e/cases.md（E2E测试用例清单）
约束     : 每个用户流程/功能至少一个用例，覆盖正常路径、错误路径、取消路径；用例要素齐全：场景名、前置条件、操作步骤（精确到控件与注入方式，如按钮/菜单/双击/右键/快捷键）、预期结果（画面状态/数据变化，可被像素断言验证）；用例的每个操作步骤必须对应一个真实可执行的入口；用例编写中发现功能无入口（需求或功能列表设计了该功能，但界面没有对应控件/菜单/快捷键）→ 返回交互模式设计或生成GUI需求分析文档补充入口，不得跳过该功能；驱动脚本按用例步骤注入操作，断言脚本按用例预期断言；脚本与用例一一对应，脚本不得包含用例清单之外的行为；需求或界面变更时先改用例清单，再同步驱动/断言脚本，保持三者一致
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : test/e2e/cases.md（E2E测试用例清单） 产出且 约束 逐条 满足；哪步不符合就跳回哪步（跳回生成GUI需求分析文档或交互模式设计）

------
## 117. 编写E2E驱动脚本

目标     : 编写PowerShell驱动脚本：等待窗口句柄就绪，按执行路径注入键盘/鼠标事件并截取客户区画面
前置     : GUI执行路径文档、test/e2e/cases.md
解构     : 观察 [等待窗口句柄就绪 / 按执行路径注入键盘/鼠标事件并截取客户区画面]
产物     : test/e2e/drive.ps1
约束     : 使用Win32 API（PostMessage）向目标窗口注入键盘/鼠标事件，不依赖UI自动化框架；脚本开头调用SetProcessDPIAware，保证GetClientRect/ClientToScreen/CopyFromScreen坐标单位一致（否则DPI虚拟化导致客户区截图黑屏）；截取客户区（排除标题栏）：标题栏激活/未激活态渲染不同，会污染稳定性对比；每个操作后等待画面稳定（动画/异步刷新完成）再截图，截图按场景命名（shot_xx.png）保存到out/verify/；KEYUP事件必须置位lParam释放沿位（bit31），否则按键状态卡住
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : test/e2e/drive.ps1 产出且 约束 逐条 满足

------
## 118. 编写E2E断言脚本

目标     : 编写像素断言脚本：基于截图颜色/区域统计，断言每个场景的真实画面状态。默认用 Python/PIL；也可用 PowerShell 内嵌 C# 实现（二选一，能力等价），与驱动脚本统一为 PowerShell 技术栈
前置     : GUI执行路径文档、GUI功能流程图、test/e2e/cases.md
解构     : 观察 [基于截图颜色/区域统计 / 断言每个场景的真实画面状态。默认用 Python/PIL；也可用 PowerShell 内嵌 C# 实现（二选一，能力等价） / 与驱动脚本统一为 PowerShell 技术栈]
产物     : test/e2e/analyze.py（默认）；可选 test/e2e/analyze.ps1（PowerShell+C#）
约束     : 默认方案（Python/PIL）：使用PIL读取截图，定义关键颜色识别函数（界面主色/强调色/文字白色等）；可选方案（PowerShell+C#）：PowerShell 用 Add-Type -TypeDefinition 内嵌 C# 代码并引用 System.Drawing；截屏用 Bitmap + Graphics.CopyFromScreen（或直接读取驱动脚本产出的 shot_xx.png）；像素读取用 Bitmap.LockBits/BitmapData 批量获取（GetPixel 仅用于少量抽查点，避免逐像素调用性能差）；按 ROI 矩形区域统计关键颜色占比，关键颜色识别函数与 Python 版等价；两种方案都必须：每个场景有明确断言项（画面非黑屏、关键元素出现/消失/位移、文案存在）；按执行路径逐场景断言（启动画面→各操作响应→完成画面），输出通过/失败汇总并以退出码返回；需求或界面变更后必须同步更新断言脚本，防止断言与实现脱节
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : test/e2e/analyze.py（默认）；可选 test/e2e/analyze.ps1（PowerShell+C#） 产出且 约束 逐条 满足

------
## 119. 编写E2E执行脚本

目标     : 编写E2E编排脚本：清理旧进程→启动程序→运行驱动脚本→关闭程序→运行断言脚本
前置     : 编译产物、test/e2e/drive.ps1、test/e2e/analyze.py
解构     : 观察 [清理旧进程→启动程序→运行驱动脚本→关闭程序→运行断言脚本]
产物     : test/e2e/verify_all.sh
约束     : 脚本顺序固定：清理→启动→驱动→关闭→分析；启动程序时重定向标准输出/错误到日志（pmem_gui_out.log/pmem_gui_err.log），便于排查；所有截图与日志输出到out/verify/目录；最终打印断言汇总（通过/失败），失败时返回非零退出码
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : test/e2e/verify_all.sh 产出且 约束 逐条 满足

------
## 120. 执行E2E测试

目标     : 执行E2E测试：真实启动程序、注入真实操作、断言真实画面
前置     : test/e2e/verify_all.sh、test/e2e/cases.md、编译产物
解构     : 观察 [真实启动程序 / 注入真实操作 / 断言真实画面]
产物     : E2E测试结果
约束     : 必须使用真实编译产物，禁止mock或伪造画面；真实抓包类用例（如实时抓包、包进入列表）需要**管理员令牌**：GUI 与驱动脚本均经 `Start-Process -Verb RunAs` 提权（UAC 弹窗点「是」后即可用；UAC「从不通知」时静默提权）；Windows 侧日常启动用 `gui/script/run_gui_admin.bat`（Release）/`run_gui_admin_debug.bat`（Debug），WSL 侧 `sudo` / `setcap cap_net_raw+ep` / `wsl -u root`。无管理员/无外部 IPv4 接口时抓包类用例标记"环境跳过"如实报告（详见 design/v<版本>/gui/33_admin_capture_guide.md）；按 test/e2e/cases.md 逐条核对用例通过/失败，输出用例级汇总；用例无法执行（入口缺失/画面不符）→ 定位到具体步骤排查；全部用例通过→继续到清理测试环境；失败→根据失败用例定位问题，跳回对应实现步骤修复后重跑
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : E2E测试结果 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 121. 清理测试环境

目标     : 杀掉测试中启动的残留GUI与target进程，防止下次测试时句柄/PID冲突
前置     : pmem_gui.exe / pmem_target.exe 运行实例（如有）
解构     : 观察 [杀掉测试中启动的残留GUI与target进程 / 防止下次测试时句柄/PID冲突]
产物     : 无残留进程
约束     : 清理命令（WSL）：taskkill.exe /f /im pmem_gui.exe 2>/dev/null; taskkill.exe /f /im pmem_target.exe 2>/dev/null; true
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 无残留进程 产出且 约束 逐条 满足

------
## 122. 根目录清理步骤

目标     : 清理根目录中散落在外的构建产物与工具目录。根目录仅允许保留：各产品源码子目录（gui/、pmem/）、design/、.flow/ 与配置文件（.gitignore、idea..txt）。CMakeLists.txt、CMakePresets.json、编译脚本、构建输出、三方依赖必须位于各自项目子目录内；只有 GUI 可执行文件发布需要时才配置 `gui/install/`，pmem 无其他依赖不得配置 `pmem/install/` 或静态库安装脚本。根目录与共享 workspace/ 均不得出现这些文件或目录
前置     : 全部编译产物、安装产物
解构     : 观察 [各产品源码子目录（gui/、pmem/）、design/、.flow/ 与配置文件（.gitignore、idea..txt）。CMakeLists.txt、CMakePresets.json、编译脚本、构建输出、三方依赖必须位于各自项目子目录内；只有 GUI 可执行文件发布需要时才配置 `gui/install/` / pmem 无其他依赖不得配置 `pmem/install/` 或静态库安装脚本。根目录与共享 workspace/ 均不得出现这些文件或目录]
产物     : 根目录无散落产物
约束     : 根目录出现 *.obj/*.exe/*.lib/*.log/*.pdb 等散落构建产物 → 删除或移入所属项目 out/（gui/out/ 或 pmem/out/）；根目录出现 CMakeLists.txt、CMakePresets.json → 删除或移入所属项目子目录；根目录出现 script/、install/、out/、third_party/ 等目录 → 删除或移入所属项目子目录；根目录出现共享 workspace/ 收纳目录 → 删除，并将其中内容按归属分发回各项目子目录；清理后依次执行项目自包含检查、根目录配置隔离检查、构建产物归属检查、三方依赖归属检查、跨项目依赖检查与独立子目录检查确认根目录整洁
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : 根目录无散落产物 产出且 约束 逐条 满足；哪步不符合就跳回哪步

------
## 123. 脚本目录清理步骤

目标     : 开发收尾时检查 script/ 目录：删除排查类与一次性工具，保留核心与验证类脚本，同步 README.md，防止临时脚本长期堆积
前置     : gui/script/ 全部脚本
解构     : 观察 [删除排查类与一次性工具 / 保留核心与验证类脚本 / 同步 README.md / 防止临时脚本长期堆积]
产物     : script/ 根目录仅剩核心编译/测试脚本
约束     : diagnose/ 下排查脚本，问题已解决且验证通过 → 删除；确需复用的移入对应分类并更新 README.md；tools/ 下补丁脚本，内容已并入流程文档 → 删除或移出归档，不再执行；verify/ 下验证脚本与 script/ 根核心编译/测试脚本 → 保留；script/ 根目录不得出现排查类/一次性脚本；新增临时脚本必须明确归入分类；清理后检查 README.md 与实际文件一致：README 列出的每个文件都存在且归属正确，实际存在的每个文件都在 README 中
排除项   : 不超出本步 目标 范围；不做 后续 步骤 的工作
完成标准 : script/ 根目录仅剩核心编译/测试脚本 产出且 约束 逐条 满足；哪步不符合就跳回哪步
