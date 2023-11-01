clc; close all; clear;

%% Simulering av sinusbølge til soft blink

syms t;
fs = 0.2;           % Frekvens [Hz]
fsRad = fs*2*pi;    % Frekvens [rad/sek]
f(t) = (1 - sin(t*fsRad))*0.5;

figure
fplot(f*100)
xlim([0, 10])
lgd = legend('$\frac{1-sin(fsRad \cdot t)}{2}$', 'Location', ...
             'southeast', 'Interpreter', 'latex');
fontsize(lgd,16,'points')
xlabel('Tid [sekunder]')
ylabel('PWM verdi [%]')
title('Soft Blink')
