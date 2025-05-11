import serial
import csv
from serial import SerialException
import time

arduino_port = "COM7"       # Porta serial do Arduino
baud = 230400 
fileName = input("Digite o nome do arquivo: ") + ".csv"

# Função para resetar o Arduino via serial
def reset_arduino(ser):
    ser.setDTR(False)  # Desliga DTR
    time.sleep(1)      # Espera 1 segundo
    ser.setDTR(True)   # Liga DTR novamente (isso causa o reset)
    time.sleep(2)      # Espera o Arduino reiniciar completamente

try:
    # Abre a conexão serial
    ser = serial.Serial(arduino_port, baud)
    
    # Reseta o Arduino antes de começar
    reset_arduino(ser)
    
    sensor_data = []  # Armazena os dados
    
    # Limpa o buffer serial (remove dados antigos)
    ser.reset_input_buffer()
    
    while True:
        try:
            getData = ser.readline()
            dataString = getData.decode('utf-8').strip()  # Remove \r\n
            readings = dataString.split(",")
            
            if len(readings) > 1:  # Verifica se recebeu dados válidos
                print(readings)
                sensor_data.append(readings)
                
        except SerialException as e:
            print(f"Erro de serial: {e}")
            break
        except UnicodeDecodeError:
            print("Erro de decodificação - pulando linha")
            continue
        except KeyboardInterrupt:
            print("Coleta interrompida pelo usuário")
            break

    # Salva os dados no arquivo CSV
    with open(fileName, 'a', encoding='UTF8', newline='') as f:
        writer = csv.writer(f)
        writer.writerows(sensor_data)
        
    print("Data collection complete!")

finally:
    # Fecha a porta serial mesmo se houver erro
    if 'ser' in locals() and ser.is_open:
        ser.close()