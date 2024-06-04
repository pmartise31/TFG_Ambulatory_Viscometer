# -*- coding: utf-8 -*-
"""
Created on %(date)s

@author: %(Pablo Martinez Serrano)s
"""
# =============================================================================
# READING THE VALUES FROM THE SERIAL MONITOR OF ARDUINO: 
# =============================================================================
import serial
import pandas as pd
import time



ser = serial.Serial('COM3', 115200)  
time.sleep(2) 

data = []

try:
    while True:
        if ser.in_waiting > 0:
            line = ser.readline().decode('utf-8').strip()
            print(f"Received: {line}")
            data.append([time.time(), line])
except KeyboardInterrupt:
    print("Data collection stopped.")
finally:
    ser.close()
    df = pd.DataFrame(data, columns=['Timestamp', 'SensorValue'])
    df.to_excel('arduino_data.xlsx', index=False)
    print("Data saved to arduino_data.xlsx")

# %%
import pandas as pd
df_data_sr = pd.DataFrame(columns=['Shear rate'])
df_data_ss = pd.DataFrame(columns=['Shear stress'])
df_data_viscosity = pd.DataFrame(columns = ['Apparent viscosity values'])
df_data_Gprima = pd.DataFrame(columns=["G'"])
df_data_G2prima = pd.DataFrame(columns=["G''"])
# %%
df_data_sr = df_data_sr.iloc[:, :-1]
df_data_ss = df_data_ss.iloc[:, :-1]
df_data_viscosity = df_data_viscosity.iloc[:, :-1]
df_data_Gprima = df_data_Gprima.iloc[:, :-1]
df_data_G2prima = df_data_G2prima.iloc[:, :-1]
#%%
# =============================================================================
#  STORAGE OF THE DATA AND CASSON MODEL FITTING CURVE   
# =============================================================================
import numpy as np
import pandas as pd
from scipy.optimize import curve_fit
def calc_parameters(datos, dataframe1, dataframe2, dataframe3, dataframe4, dataframe5):
    df_data_shear_rate = pd.DataFrame(columns=['Exp_'+str(dataframe1.shape[1]+1)])
    df_data_shear_stress = pd.DataFrame(columns=['Exp_'+str(dataframe2.shape[1]+1)])
    df_data_viscosity_app = pd.DataFrame(columns=['Exp_'+str(dataframe3.shape[1]+1)])
    df_data_Gprima = pd.DataFrame(columns=['Exp_'+str(dataframe4.shape[1]+1)])
    df_data_G2prima = pd.DataFrame(columns=['Exp_'+str(dataframe5.shape[1]+1)])
    shear_rate = []
    shear_stress=[]
    viscosity = []
    for i in range(1, len(datos)-101):
        if i % 2 != 0: 
            shear_stress.append(float(datos[i][1]))
        else: 
            shear_rate.append(float(datos[i][1]))
    tau_0 = datos[-100][1]
    print(tau_0)
    mu = datos[-99][1]
    print(mu)
    G_prime = datos[-98][1]
    print(G_prime)
    G_2prime = datos[-97][1]
    print(G_2prime)
    viscosity=datos[-96:-64]
    Gprima=datos[-64:-32]
    G2prima=datos[-33:None]
    viscs= []
    Gprimas = []
    G2primas = []
    for i in range(len(viscosity)):
        viscs.append(float(viscosity[i][1]))
        Gprimas.append(float(Gprima[i][1]))
        G2primas.append(float(G2prima[i][1]))
    shear_rate = np.array(shear_rate)
    shear_stress = np.array(shear_stress)
    viscosity = np.array(viscosity)
    df_data_shear_rate['Exp_'+str(dataframe1.shape[1]+1)] = shear_rate
    df_data_shear_stress['Exp_'+str(dataframe2.shape[1]+1)] = shear_stress
    df_data_viscosity_app['Exp_'+str(dataframe3.shape[1]+1)] = viscs
    df_data_Gprima['Exp_'+str(dataframe4.shape[1]+1)] = Gprimas
    df_data_G2prima['Exp_'+str(dataframe5.shape[1]+1)] = G2primas
    dataframe1= pd.concat([dataframe1,df_data_shear_rate], axis= 1)
    dataframe2= pd.concat([dataframe2,df_data_shear_stress], axis = 1)
    dataframe3= pd.concat([dataframe3,df_data_viscosity_app], axis = 1)
    dataframe4= pd.concat([dataframe4,df_data_Gprima], axis = 1)
    dataframe5= pd.concat([dataframe5,df_data_G2prima], axis = 1)
    return dataframe1, dataframe2, dataframe3, dataframe4, dataframe5

df_data_sr, df_data_ss, df_data_viscosity, df_data_Gprima, df_data_G2prima = calc_parameters(data, df_data_sr, df_data_ss, df_data_viscosity, df_data_Gprima, df_data_G2prima)

    

#%%
# =============================================================================
#  CURVES FOR THE SHEAR STRESS VS SHEAR RATE
# =============================================================================
import matplotlib.pyplot as plt
from scipy.interpolate import make_interp_spline


shear_rate_vals = []
shear_stress_vals = []
for u in range(df_data_sr.shape[1]):
    shear_rate_list = df_data_sr['Exp_'+str(u+1)].tolist() 
    shear_stress_list = df_data_ss['Exp_'+str(u+1)].tolist()
    shear_rate_vals.append(shear_rate_list)
    shear_stress_vals.append(shear_stress_list)

# Configurar el estilo del gráfico
plt.figure(figsize=(10, 6))

# Colores y estilos para las líneas, más desenfadados
colors = ['#FF5733', '#FFC300', '#C70039', '#900C3F', '#581845', '#00FF00', '#00FFFF']
linestyles = ['-', '--', '-.', ':', '-', '--', '-']
speeds = [500, 750, 960]

# Iterar sobre las mediciones y trazar cada una
for i in range(df_data_sr.shape[1]):
    # Aplicar interpolación spline cúbica
    x_smooth = np.linspace(min(shear_rate_vals[i]), max(shear_rate_vals[i]), 200)
    y_smooth = make_interp_spline(shear_rate_vals[i], shear_stress_vals[i])(x_smooth)
    
    plt.plot(x_smooth, y_smooth, label=f'v = {speeds[i]} um/s', color=colors[i%len(colors)])

# Configuraciones adicionales del gráfico con Comic Sans
plt.title('Shear Stress vs. Shear Rate for Carbopol at 0.1 w/w', fontsize=16, fontweight='bold', fontname='Times New Roman')
plt.xlabel('Shear Rate (s$^{-1}$)', fontsize=14, fontname='Times New Roman')
plt.ylabel('Shear Stress (Pa)', fontsize=14, fontname='Times New Roman')
plt.legend()
plt.grid(True, which='both', linestyle='--', linewidth=0.5)

# Ajustar el diseño para que quede más bonito
plt.tight_layout()
# Mostrar el gráfico
plt.show()


# %%
# =============================================================================
# CASSON'S MODEL: FITTING CASSON'S MODEL WITH THE EXPERIMENTAL RESULTS:
# =============================================================================

def casson_model(shear_rate, tau_y, eta_p):
    return (tau_y**0.5 + (eta_p * shear_rate)**0.5)**2

df_sr = df_data_sr['Exp_1'].drop(df_data_sr['Exp_1'].index[:5])
df_ss = df_data_ss['Exp_1'].drop(df_data_ss['Exp_1'].index[:5])

popt, pcov = curve_fit(casson_model, df_sr, df_ss, p0=[150, 1])
tau_y_estimated = popt[0]
shear_rate_fit = np.linspace(min(df_sr), max(df_sr), 100)
shear_stress_fit = casson_model(shear_rate_fit, *popt)
plt.figure(figsize=(12, 8))
plt.scatter(df_sr, df_ss, label='Experimental data', color='blue', marker='o')
plt.plot(shear_rate_fit, shear_stress_fit, label='Adjusted curve', color='red', linestyle='--')
plt.title("Casson's model adjustment", fontsize=16)
plt.xlabel('Shear Rate (1/s)', fontsize=14)
plt.ylabel('Shear Stress (Pa)', fontsize=14)
plt.legend(fontsize=12)
plt.grid(True, which='both', linestyle=':', linewidth=0.5)
plt.tight_layout()
plt.show()


# %%
import matplotlib.pyplot as plt
from scipy.interpolate import make_interp_spline


shear_rate_vals = []
viscosity_vals = []
for u in range(df_data_sr.shape[1]):
    shear_rate_list = df_data_sr['Exp_'+str(u+1)].tolist() 
    viscosity_list = df_data_viscosity['Exp_'+str(u+1)].tolist()
    shear_rate_vals.append(shear_rate_list)
    viscosity_vals.append(viscosity_list)

# Configurar el estilo del gráfico
plt.figure(figsize=(10, 6))

# Colores y estilos para las líneas, más desenfadados
colors = ['#FF5733', '#FFC300', '#C70039', '#900C3F']
linestyles = ['-', '--', '-.', ':', '-', '--', '-']
speeds = [300, 500, 700, 960]
concentrations = [0.15,0.2,0.25,0.3]

# Iterar sobre las mediciones y trazar cada una
fig, ax = plt.subplots()
for j in range(len(colors)):
    for i in range(df_data_sr.shape[1]):
        # Aplicar interpolación spline cúbica
        x_smooth = np.linspace(min(shear_rate_vals[i]), max(shear_rate_vals[i]), 200)
        y_smooth = make_interp_spline(shear_rate_vals[i], viscosity_vals[i])(x_smooth)
        
        plt.plot(x_smooth, y_smooth, label=f'v = {speeds[j]} um/s, c = {concentrations[j]} w/w', color=colors[j%len(colors)], linestyle = linestyles[j])

# Configuraciones adicionales del gráfico con Comic Sans
plt.title('Viscosity vs. Shear Rate for Carbopol at 0.15 w/w', fontsize=16, fontweight='bold', fontname='Times New Roman')
plt.xlabel('Shear Rate (s$^{-1}$)', fontsize=14, fontname='Times New Roman')
plt.ylabel('Viscosity (Pa·s)', fontsize=14, fontname='Times New Roman')
plt.xlim([-20,1000])
plt.legend()
plt.grid(True, which='both', linestyle='--', linewidth=0.5)

# Ajustar el diseño para que quede más bonito
plt.tight_layout()
# Mostrar el gráfico
plt.show()
# %%



# %% 
from plotnine import ggplot, aes, geom_line, labs, scale_color_manual, scale_linetype_manual, theme, guides, guide_legend, xlim
from scipy.interpolate import make_interp_spline
from plotnine.themes import element_text

shear_rate_vals = []
viscosity_vals = []
speeds = [300, 500, 700, 960]
concentrations = [0.15, 0.2, 0.25, 0.3]
colors = ['#FF5733', '#3CAEA3', '#FFC300', '#6A4C93']
mar = ['-', '--', '-.', ':']

for u in range(df_data_sr.shape[1]):
    shear_rate_list = df_data_sr['Exp_' + str(u + 1)].tolist()
    viscosity_list = df_data_viscosity['Exp_' + str(u + 1)].tolist()
    
    x_smooth = np.linspace(min(shear_rate_list), max(shear_rate_list), 200)
    y_smooth = make_interp_spline(shear_rate_list, viscosity_list)(x_smooth)
    
    shear_rate_vals.append(x_smooth)
    viscosity_vals.append(y_smooth)

speeds2 = np.array([])
for u in range(len(speeds)):
    for i in range(len(speeds)):
        speeds1 = np.repeat(speeds, 200)
    speeds2 = np.concatenate((speeds2,speeds1))

data = {
    'Shear Rate': np.concatenate(shear_rate_vals),
    'Viscosity': np.concatenate(viscosity_vals),
    'Speed': speeds2,
    'Concentration': np.repeat(concentrations, 800)
}

df_plot = pd.DataFrame(data)

plot = (
    ggplot(df_plot, aes(x='Shear Rate', y='Viscosity', color='factor(Concentration)', linetype='factor(Speed)')) +
    geom_line(size=1.2) +
    labs(title='Viscosity vs. Shear Rate for Carbopol at different concentrations and speeds',
         x='Shear Rate (s$^{-1}$)',
         y='Viscosity (Pa·s)',
         color='Speed (um/s)',
         linetype='Concentration (w/w)') +
    scale_color_manual(values=colors) +
    scale_linetype_manual(values=linestyles) +
    theme(
        figure_size=(10, 6),
        plot_title=element_text(hjust=0.5, size=16, weight='bold'),  # Centrar el título
        legend_position='top'
    ) +
 guides(color=guide_legend(title='Concentration (w/w)'), linetype=guide_legend(title='Speed (um/s)')) +
    xlim(55, 250)  # Limitar el eje X hasta 1000
)

print(plot)

# %%

df_data_sr.to_csv(r"C:\Users\pms20\OneDrive\Escritorio\UB things\TFG\Plots\Shear_rate.csv")
df_data_ss.to_csv(r"C:\Users\pms20\OneDrive\Escritorio\UB things\TFG\Plots\Shear_stress.csv")
df_data_viscosity.to_csv(r"C:\Users\pms20\OneDrive\Escritorio\UB things\TFG\Plots\Viscosity.csv")

# %% 
# =============================================================================
# CHEKING THE EQUIPMENT PERFORMS MEASUREMENTS ACCURATELY
# =============================================================================
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy.interpolate import make_interp_spline 

def mean_squared_error(list1, list2):
    list1 = np.array(list1)
    list2 = np.array(list2)
    mse = np.mean((list1 - list2) ** 2)
    return mse

def device_calib(dataframe1, dataframe2, dataframe3, dataframe4, dataframe5, tau_0):
    visc_teor = []
    G_teor = []
    G2_teor = []
    for r in range(len(dataframe1)):
        visc = ((np.sqrt(dataframe1['Exp_1'][r])-np.sqrt(tau_0))**2)/dataframe2['Exp_1'][r]
        Gprima = tau_0/dataframe2['Exp_1'][r]
        G2prima = dataframe1['Exp_1'][r]/dataframe2['Exp_1'][r]
        visc_teor.append(visc)
        G_teor.append(Gprima)
        G2_teor.append(G2prima)
    viscs_exp = dataframe3['Exp_1'].tolist()
    G_exp = dataframe4['Exp_1'].tolist()
    G2_exp = dataframe5['Exp_1'].tolist()
    MSE_visc = mean_squared_error(visc_teor, viscs_exp)
    mean_MSE_visc = np.mean(MSE_visc)
    MSE_G = mean_squared_error(G_teor, G_exp)
    mean_MSE_G = np.mean(MSE_G)
    MSE_G2 = mean_squared_error(G2_teor, G2_exp)
    mean_MSE_G2 = np.mean(MSE_G2)
    df_viscs_exp = pd.DataFrame(viscs_exp, columns=['Experimental'])
    df_viscs_teor = pd.DataFrame(visc_teor, columns=['Theoretical'])
    df_G_exp = pd.DataFrame(G_exp, columns=['Experimental'])
    df_G_teor = pd.DataFrame(G_teor, columns=['Theoretical'])
    df_G2_exp = pd.DataFrame(G2_exp, columns=['Experimental'])
    df_G2_teor = pd.DataFrame(G2_teor, columns=['Theoretical'])
    plt.figure(1) 
    plt.scatter(dataframe2['Exp_1'].tail(31), df_viscs_exp['Experimental'].tail(31), color='blue', label= 'Experimental data')
    plt.plot(dataframe2['Exp_1'].iloc[:-1],df_viscs_teor['Theoretical'].iloc[:-1], color = 'red', label = 'Theoretical results')
    plt.title('Apparent viscosity comparison')
    plt.xlabel('Shear rate (s-1)')
    plt.ylabel('Apparent viscosity (Pa·s)')
    plt.legend()
    plt.show()
    plt.tight_layout()
    plt.figure(2) 
    plt.scatter(dataframe2['Exp_1'].tail(31), df_G_exp['Experimental'].tail(31), color='blue', label= 'Experimental data')
    plt.plot(dataframe2['Exp_1'].iloc[:-1],df_G_teor['Theoretical'].iloc[:-1], color = 'red', label = 'Theoretical results')
    plt.title('Storage modulus comparison')
    plt.xlabel('Shear rate (s-1)')
    plt.ylabel("G' (Pa·s)")
    plt.legend()
    plt.show()
    plt.tight_layout()
    plt.figure(3) 
    plt.scatter(dataframe2['Exp_1'].tail(30), df_G2_exp['Experimental'].tail(30), color='blue', label= 'Experimental data')
    plt.plot(dataframe2['Exp_1'].iloc[:-1],df_G2_teor['Theoretical'].iloc[:-1], color = 'red', label = 'Theoretical results')
    plt.title('Loss modulus comparison')
    plt.xlabel('Shear rate (s-1)')
    plt.ylabel("G'' (Pa·s)")
    plt.legend()
    plt.show()
    plt.tight_layout()
    return mean_MSE_visc/(max(dataframe3['Exp_1'])-min(dataframe3['Exp_1'])), mean_MSE_G/(max(dataframe4['Exp_1'])-min(dataframe4['Exp_1'])), mean_MSE_G2/(max(dataframe5['Exp_1'])-min(dataframe5['Exp_1']))
device_calib(df_data_ss, df_data_sr, df_data_viscosity, df_data_Gprima, df_data_G2prima, 50)
