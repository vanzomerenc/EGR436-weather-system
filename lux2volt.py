from math import *

def lux2volt(lux, *, R=330, V=3.3):
    """
    Based on the parameters of the light sensor we are using,
    and this configuration:
    
    V+ |--[_R_]--.--(_SENSOR_)--| GND
                 |
                 V
            PORT_X:PIN_Y
    """
    return V * R / (R + 33000 * 5.6**(-log10(lux/10)))

for i in range(1, 7):
    print("Lux: {}, Volts: {}".format(10**i, lux2volt(10**i, R=)))

