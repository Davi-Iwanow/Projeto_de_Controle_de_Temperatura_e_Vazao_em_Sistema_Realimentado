clc; clear;
close all;

%% Determinacao da Funcao de Transferencia G_T(s) relaciona a temperatura a tensao aplicada na resistencia

filenameT = 'dados_temperatura.csv';              % Arquivo com dados medidos
data = readmatrix(filenameT);                     % Lê os dados do arquivo CSV

tempo = data(:,1)/1000;                           % Tempo em segundos
temperatura = data(:,2);                          % Temperatura em °C

length_ut = length(tempo);
kt = 3;                                           % Número de elementos iniciais iguais a zero

ut = [zeros(1, kt), 255*ones(1, length_ut - kt)]';    % Vetor de entrada (pwm 0/1) transposta (em colunas)

dados = iddata(temperatura,ut,1.658);             % Vetores coluna
ft_ident = tfest(dados,1,0);

numt = ft_ident.Numerator;
dent = ft_ident.Denominator;

G_modelo_t = tf(numt,dent);                       % Função de Transferência (TEMPERATURA)

%% Gráficos Temperatura

y_mt = lsim(G_modelo_t,ut,tempo);
figure(1);
plot(tempo,temperatura);
hold on
plot(tempo,y_mt)
legend('Real','Modelo')
xlabel('Tempo (s)');
ylabel('Temperatura Ajustada (°C)');
title('Resposta ao Degrau da Temperatura');
legend('Dados Experimentais', 'Modelo Ajustado');
grid on;


%% Determinacao da Funcao de Transferencia G_F(s) relaciona a vazao e a tensao aplicada no cooler

filenameF = 'vazao5.csv';                         % Arquivo com dados medidos
data = readmatrix(filenameF);                     % Lê os dados do arquivo CSV

tempo_f = data(:,1)/1000;                         % Tempo em segundos
vazao = data(:,2);                                % Vazao em pulsos

length_uf = length(tempo_f);
kf = 1;                                           % Número de elementos iniciais iguais a zero

uf = [zeros(1, kf), 255*ones(1, length_uf -kf)]';

dados = iddata(vazao,uf,0.500);                   % Vetores coluna
ft_ident = tfest(dados,1,0);

numf = ft_ident.Numerator;
denf = ft_ident.Denominator;

G_modelo_f = tf(numf,denf);                       % Função de Transferência (VAZÃO)

%% Gráficos Vazão

y_mf = lsim(G_modelo_f,uf,tempo_f);
figure(2);
plot(tempo_f,vazao);
hold on
plot(tempo_f,y_mf)
legend('Real','Modelo')
xlabel('Tempo (s)');
ylabel('Vazao (Pulsos)');
title('Resposta ao Degrau da Vazao');
legend('Dados Experimentais', 'Modelo Ajustado');
grid on;

stepinfo(G_modelo_f);

%% Determinacao da Funcao de Transferencia G_TF(s) relaciona a Temperatura a Perturbacao gerada pelo cooler

filenameTF = 'dados_perturbacao.csv';             % Arquivo com dados medidos
data = readmatrix(filenameTF);                    % Lê os dados do arquivo CSV

t = data(:,1);                                    % Tempo em milisegundos
tempo_p = (t(274:end) - 456500)/1000;             % Vetor correto de tempo em segundos
temp = data(:,2);                                 % Temperatura em °C
temperatura_p = temp(274:end) - 80;               % Vetor correto de temperatura desconsiderando o valor inicial

length_u = length(tempo_p);
k = 314-274;                                      % Número de elementos iniciais iguais a zero

u = [zeros(1, k), 255*ones(1, length_u  -k)];
u = u';                                           % Matriz transposta para formacao do vetor correto
dados = iddata(temperatura_p,u,1.660);            % Vetores coluna
ft_ident = tfest(dados,1,0);

num = ft_ident.Numerator;
den = ft_ident.Denominator;

G_modelo = tf(num,den);                           % Funcao de Transferência (TEMPERATURA com PERTURBAÇÃO)

%% Gráficos da Temperatura com Perturbação

y_m = lsim(G_modelo,u,tempo_p);
figure(3);
plot(tempo_p,temperatura_p);
hold on
plot(tempo_p,y_m);
legend('Real','Modelo')
xlabel('Tempo (s)');
ylabel('Temperatura Ajustada (°C)');
title('Resposta ao Degrau da Temperatura com Perturbacao');
legend('Dados Experimentais', 'Modelo Ajustado');
grid on;


%% Discretização das Plantas

% Planta Temperatura
Ta_temp = 1.658;
G_t_disc = c2d(G_modelo_t, Ta_temp, 'zoh');

% Planta Vazão
Ta_vazao = 0.5;
G_f_disc = c2d(G_modelo_f, Ta_vazao, 'zoh');

% Planta Temperatura com perturbacao de Vazao
Ta_perturbacao = 1.660;
G_tf_disc = c2d(G_modelo, Ta_perturbacao, 'zoh');


%% Gráficos Continuo x Discreto

% Temperatura
figure(4)
step(255*G_modelo_t)
hold on
step(255*G_t_disc)
title('Continuo x Discreto - Temperatura');
legend("Contínuo", "Discreto")
grid on;

% Vazao
figure(5)
step(255*G_modelo_f)
hold on
step(255*G_f_disc)
title('Continuo x Discreto - Vazao');
legend("Contínuo", "Discreto")
grid on;

% Temperatura perturbada
figure(6)
step(255*G_modelo)
hold on
step(255*G_tf_disc)
title('Continuo x Discreto - Temperatura com Perturbacao');
legend("Contínuo", "Discreto")
grid on;

%% Controladores

% sisotool(G_f_disc)
% sisotool(G_t_disc)

C_t_disc = zpk([0.995], [1], 4.097, Ta_temp);
C_f_disc = zpk([0.792],[1],1.72,Ta_vazao);