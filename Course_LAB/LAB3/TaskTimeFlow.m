% ======================
% RMS 调度任务时间线绘图
% ======================

clear; close all; clc;

% 任务参数
tasks = {'A','B','C'};
period = [1000 1500 2800];
exec   = [200  700  300 ];

% 定义颜色（与示例接近）
colors = [1   0.5  0.5;     % A 红
          1   0.6  0.1;     % B 橙
          0.2 0.8 0.2];     % C 绿

% 手工调度结果（根据分析得出的 CPU 时间片）
schedule = [
    0   200   1;
    200 900   2;
    900 1000  3;
    1000 1200 1;
    1200 1400 3;
    1500 2000 2;
    2000 2200 1;
    2200 2400 2;
    2800 3000 3;
    3000 3200 1;
    3200 3900 2;
    3900 4000 3;
    4000 4200 1;
];
figure('Position',[100 100 1200 350]); hold on; box on;
title('Rate Monotonic Scheduling Timeline','FontSize',14);
xlabel('Time (ms)'); ylabel('Task');
yticks([1 2 3]); yticklabels({'Task A','Task B','Task C'});

for i = 1:size(schedule,1)
    t_start = schedule(i,1);
    t_end   = schedule(i,2);
    task_id = schedule(i,3);
    rectangle('Position',[t_start, task_id-0.4, t_end-t_start, 0.8], ...
              'FaceColor',colors(task_id,:), 'EdgeColor','none');
    text((t_start+t_end)/2, task_id, sprintf('%s',tasks{task_id}), ...
         'HorizontalAlignment','center','VerticalAlignment','middle',...
         'FontWeight','bold','Color','k');
end

% 画任务释放点
for i = 1:5  
    xline((i-1)*period(1),'--r');
end
for i = 1:4
    xline((i-1)*period(2),'--','Color',[1 0.6 0]);
end
for i = 1:3
    xline((i-1)*period(3),'--g');
end

xlim([0 4500]);
ylim([0.5 3.5]);

hold off;




%% =============================
%   RMS Scheduling with Resource Access
% =============================
clear; close all; clc;

% Task params
tasks = {'A','B','C'};
period = [1000 1500 2800];
exec   = [200  700  300 ];
colors = [1   0.5  0.5;     % A 红
          1   0.6  0.1;     % B 橙
          0.2 0.8 0.2];     % C 绿

% 手工调度结果（含资源访问 + 优先级继承）
% 列: [开始时间, 结束时间, 任务ID]
schedule = [
    0    200   1;    % A0
    200  900   2;    % B0
    900  1000  3;    % C0 前100ms
    1000 1200  3;    % C0 占资源200ms
    1200 1400  1;    % A1
    1500 2000  2;    % B1 前500ms
    2000 2200  1;    % A2
    2200 2400  2;    % B1 剩余200ms? 修正: 实际剩余200ms，执行2200–2400
    2800 2900  3;    % C1 前100ms
    2900 3100  3;    % C1 占资源200ms
    3100 3300  1;    % A3
    3300 4000  2;    % B2
    4000 4200  1;    % A4
];

% 标记资源使用段
resource_use = [
    1000    1200   3;    % C0
    2900    3100   3;    % C1
];


figure('Position',[100 100 1300 350]); hold on;

title('Rate Monotonic Scheduling with Resource Access','FontSize',14);
xlabel('Time (ms)'); ylabel('Task');
yticks([1 2 3]); yticklabels({'Task A','Task B','Task C'});

% 画任务时间块
for i = 1:size(schedule,1)
    t1 = schedule(i,1); t2 = schedule(i,2);
    id = schedule(i,3);
    rectangle('Position',[t1 id-0.4 t2-t1 0.8], ...
        'FaceColor', colors(id,:), 'EdgeColor','k');
    text((t1+t2)/2, id, tasks{id}, 'HorizontalAlignment','center','FontWeight','bold');
end


for i = 1:size(resource_use,1)
    x = resource_use(i,:);
    rectangle('Position',[x(1), x(3)-0.50, x(2)-x(1), 0.1], 'FaceColor','k','EdgeColor','none');
end

% 绘制任务周期虚线
for i = 0:4, xline(i*period(1),'--r'); end
for i = 0:3, xline(i*period(2),'--','Color',[1 0.6 0]); end
for i = 0:2, xline(i*period(3),'--g'); end


xline(2800,'--g');


ylim([0.3 3.5]);
xlim([0 4500]);                    % 设置 X 轴范围

% 手动设置 X 轴刻度
xticks([0 500 1000 1500 2000 2500 2800 3000 3500 4000 4500]);
xticklabels({'0','500','1000','1500','2000','2500','2800','3000','3500','4000','4500'});

grid on;


% ======================
% RMS 调度任务时间线绘图
% ======================

clear; close all; clc;

% 任务参数
tasks = {'A','B','C'};
period = [1000 1500 2800];
exec   = [200  700  300 ];

% 定义颜色（与示例接近）
colors = [1   0.5  0.5;     % A 红
          1   0.6  0.1;     % B 橙
          0.2 0.8 0.2];     % C 绿

% 手工调度结果（根据分析得出的 CPU 时间片）
schedule = [
    0   200   1;
    200 900   2;
    900 1200  3;
    1200 1400 1;
    1500 2000 2;
    2000 2200 1;
    2200 2400 2;
    2800 3000 3;
    3000 3700 2;
    3700 3800 3;
    3800 4000 1;
    4000 4200 1;
];
figure('Position',[100 100 1200 350]); hold on; box on;
title('Rate Monotonic Scheduling Timeline','FontSize',14);
xlabel('Time (ms)'); ylabel('Task');
yticks([1 2 3]); yticklabels({'Task A','Task B','Task C'});

for i = 1:size(schedule,1)
    t_start = schedule(i,1);
    t_end   = schedule(i,2);
    task_id = schedule(i,3);
    rectangle('Position',[t_start, task_id-0.4, t_end-t_start, 0.8], ...
              'FaceColor',colors(task_id,:), 'EdgeColor','none');
    text((t_start+t_end)/2, task_id, sprintf('%s',tasks{task_id}), ...
         'HorizontalAlignment','center','VerticalAlignment','middle',...
         'FontWeight','bold','Color','k');
end

% 画任务释放点
for i = 1:5  
    xline((i-1)*period(1),'--r');
end
for i = 1:4
    xline((i-1)*period(2),'--','Color',[1 0.6 0]);
end
for i = 1:3
    xline((i-1)*period(3),'--g');
end

xlim([0 4500]);
ylim([0.5 3.5]);
% 手动设置 X 轴刻度
xticks([0 500 1000 1500 2000 2500 2800 3000 3500 4000 4500]);
xticklabels({'0','500','1000','1500','2000','2500','2800','3000','3500','4000','4500'});

hold off;
