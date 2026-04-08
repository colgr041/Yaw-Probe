%% Reader and plotter of the .csv
one_sensor = 'prova.txt';

data = readmatrix(one_sensor, 'NumHeaderLines', 20);

time = data(:,1);
pressure = data(:,2);

%% Butterwhorth filter
Fs = 10; % Sampling frequency
Fc = 0.5; % Cutoff frequency (val od frequency that acts like treshlod
Wn = Fc/(Fs/2); % Normalize frequency 
[b, a] = butter(6, Wn, 'low'); % 4 is the order of the filter , b and a are transfer functions coefficients
y = filtfilt(b, a, pressure);
% y = movmean(pressure, 3); % moving average filter

% Plot without filter
figure;
plot(time, pressure, '-o', 'DisplayName','Pressure difference');
ylim([30 40]);
grid on
xlabel('Time [ms]');
ylabel('Pressure difference [Pa]');
%title('Lift polar');
legend

% Plot with filter
figure;
plot(time, y, '-o', 'DisplayName','Pressure difference');
ylim([30 40]);
grid on
xlabel('Time [ms]');
ylabel('Pressure difference [Pa]');
%title('Lift polar');
legend