
% Puerto serie (bluetooth)
delete(instrfind({'Port'},{'COM8'}));
s = serial('COM8','BaudRate',9600,'Terminator','CR/LF');
fopen(s);

% Configuración de las gráficas en tiempo real
figure(1);
subplot(2,1,1)
title('Pitch angle')
xlabel("time")
ylabel("deg")
p1 = animatedline;
p1.Color = '0.00,0.00,1.00';
p1.LineWidth = 3;
ax = gca;
ax.XGrid = 'on';
ax.YGrid = 'on';
ax.XMinorGrid = 'on';
ax.YMinorGrid = 'on';
ax.YLim = [-90 90];
ax.YTick = [-90,-45,0,45,90];
ax.GridColor = '0.0,0.0,0.0';
ax.GridAlpha = 0.3;

subplot(2,1,2)
title('Roll angle')
xlabel("time")
ylabel("deg")
p2 = animatedline;
p2.Color = '0.00,1.00,0.00';
p2.LineWidth = 3;
ax = gca;
ax.XGrid = 'on';
ax.YGrid = 'on';
ax.XMinorGrid = 'on';
ax.YMinorGrid = 'on';
ax.YLim = [-90 90];
ax.YTick = [-90,-45,0,45,90];
ax.GridColor = '0.0,0.0,0.0';
ax.GridAlpha = 0.3;



% Contamos el tiempo transcurrido:
startTime = datetime('now');

% Loop
while 1==1
    
    % Lectura de los datos:
    a = fscanf(s,'%f %f ')';
    disp(a);

    % Mostrar los datos en las gráficas de los últimos 30 seg:

    % Pitch
    subplot(2,1,1);
    ax = gca;
    t = datetime('now') - startTime;
    addpoints(p1,datenum(t),a(1));
    ax.XLim = datenum([t-seconds(30) t]);
    datetick('x','keeplimits');
    drawnow;
    
    % Roll
    subplot(2,1,2);
    ax = gca;
    t = datetime('now') - startTime;
    addpoints(p2,datenum(t),a(2));
    ax.XLim = datenum([t-seconds(30) t]);
    datetick('x','keeplimits');
    drawnow;
    clear a;

end

