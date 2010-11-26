import sys
import getopt
import binascii
import socket
import time

def menu_script():
   print 'Nao foi passado nenhum pararetro na linha de comando'

#############  Funcoes de checagem dos parametros #########################
def check_size_id(idboard):
   if(len(idboard) != 4):
      print '**************************** ERRO **************************************'
      print "*** Erro: O LAC deve possuir 4 digitos hexadecimais apenas***"
      print '************************************************************************'
      sys.exit();

def check_size_dtmf(dtmf):
   if(len(dtmf) != 2):
      print '**************************** ERRO **************************************'
      print "*** Erro: O LAC deve possuir 2 digitos hexadecimais apenas***"
      print '************************************************************************'
      sys.exit();

def check_val_of_band(banda):
   for k in range(len(banda)):
      if(banda[k] != '800' and banda[k] != '850' and 
	 banda[k] != '900' and banda[k] != '1800' and
	 banda[k] != '1900' and banda[k] != '2100' and 
	 banda[k] != '2400'):
	 print '**************************** ERRO **************************************'
	 print banda[k] + ' nao eh uma banda valida! [800, 850, 900, 1800, 1900, 2100, 2400]'
	 print '************************************************************************'
	 sys.exit()

def check_val_of_status(status):
   for k in range(len(status)):
      if(status[k] != '0' and status[k] != '1'): 
	 print '**************************** ERRO **************************************'
	 print status[k] + ' nao eh um valor valido! [0/ Desligar         1/Ligar]'
	 print '************************************************************************'
	 sys.exit()


def check_val_of_pots(pots):
   for k in range(len(pots)):
      if(pots[k] < '0' or pots[k] > '30'): 
	 print '**************************** ERRO **************************************'
	 print pots[k] + ' eh um valor valido! [Minimo: 0       Maximo: 30]'
	 print '************************************************************************'
	 sys.exit()
   print pots
###########################################################################

def supp_on_dbfc(iddbfc):
   data_hex = binascii.a2b_hex("0012000266FBE99F404BE99F40A31F" + iddbfc + '01')
   s.send(data_hex)



def supp_off_dbfc(iddbfc):
   data_hex = binascii.a2b_hex("0012000266FBE99F404BE99F40A31F" + iddbfc + '00')
   s.send(data_hex)



def supp_on_dbfj(iddbf):
   data_hex = binascii.a2b_hex("001200026BFBE99F404BE99F40A31F" + iddbfj + '01')
   s.send(data_hex)



def supp_off_dbfj(iddbfj):
   data_hex = binascii.a2b_hex("001200026BFBE99F404BE99F40A31F" + iddbfj + '00')
   s.send(data_hex)

def supp_send_dtmf(dtmf):
   data_hex = binascii.a2b_hex("001000020000000001000000010000" + dtmf)
   s.send(data_hex)

def supp_add_imei():

   tipo = '00'
   ramal = '0000'
   idxx = '0234'
   idxxx = '0235'
   idxxxx = '0236'
   imei  = '32323334353637383930'
   contador1 = 0
   contador2 = 0
   contador3 = 0
   t = 0
   while t < 2:
      contador1 = contador1 + 1

      if(contador1 == 9):
	 contador2 = contador2 + 1
	 
      if(contador2 == 9):
	 contador3 = contador3 + 1
	   
      sufixo = '' 
      sufixo = '31' + '31' + '3' + str(contador3) + '3' + str(contador2) + '3' + str(contador1) + '00'
	
      if(contador1 == 9):
	 contador1 = 0

      if(contador2 == 9):
	 contador2 = 0
	
      if(contador3 == 9):
	 contador3 = 0
	   
      print "0024000264FBE99F404BE99F40A31F" + idxx + tipo + imei + sufixo + ramal
      data_hex = binascii.a2b_hex("0024000264FBE99F404BE99F40A31F" + idxx + tipo + imei + sufixo + ramal)
      s.send(data_hex)
      data_hex = binascii.a2b_hex("0024000264FBE99F404BE99F40A31F" + idxxx + tipo + imei + sufixo + ramal)
      s.send(data_hex)
      data_hex = binascii.a2b_hex("0024000264FBE99F404BE99F40A31F" + idxxxx + tipo + imei + sufixo + ramal)
      s.send(data_hex)
      t = t + 1


def supp_rem_imei():
	tipo = '02'
	ramal = '0000'
	idxx = '0234'
	idxxx = '0235'
	idxxxx = '0236'
	imei  = '31323334353637383930'
	contador1 = 0
	contador2 = 0
	contador3 = 0
	t = 0
	while t < 100:
		t = t + 1
		if(contador1 == 9):
		   	contador2 = contador2 + 1
			
		if(contador2 == 9):
		   	contador3 = contador3 + 1
			
      		if(contador3 == 9):
	 		contador3 = 0

		sufixo = ''
		sufixo = '31' + '31' + '3' + str(contador3) + '3' + str(contador2) + '3' + str(contador1) + '00'
		
		if(contador1 == 9):
		      	contador1 = 0
		      
		if(contador2 == 9):
			contador2 = 0
	
		if(contador3 == 9):
			contador3 = 0
	   
		print "0024000264FBE99F404BE99F40A31F" + idxx + tipo + imei + sufixo + ramal
   		data_hex = binascii.a2b_hex("0024000264FBE99F404BE99F40A31F" + idxx + tipo + imei + sufixo + ramal)
		s.send(data_hex)
		data_hex = binascii.a2b_hex("0024000264FBE99F404BE99F40A31F" + idxxx + tipo + imei + sufixo + ramal)
		s.send(data_hex)
  		data_hex = binascii.a2b_hex("0024000264FBE99F404BE99F40A31F" + idxxxx + tipo + imei + sufixo + ramal)
		s.send(data_hex)
		contador1 = contador1 + 1

def atualiza_lista_imei():
	iddbfc = '0213'
	header = "00026AFBE99F404BE99F40A31F"
	pack = ''
	ramal = '0000'
	imei  = '31323334353637383930'
	imsi  = '37323434353637383930'
	totalimei = 200
	totalimsi = 200
	packimei = ''
	packimsi = ''
	contador1 = 0
	contador2 = 0
	contador3 = 0
	t = 0
	while t < totalimei:
		t = t + 1
		
		if(contador1 == 9):
		   	contador2 = contador2 + 1
			
		if(contador2 == 9):
		   	contador3 = contador3 + 1
			
      		if(contador3 == 9):
	 		contador3 = 0

		sufixo = ''
		sufixo = '31' + '31' + '3' + str(contador3) + '3' + str(contador2) + '3' + str(contador1) + '00'
		
		if(contador1 == 9):
		      	contador1 = 0
		      
		if(contador2 == 9):
			contador2 = 0
	
		if(contador3 == 9):
			contador3 = 0

		packimei = packimei + imei + sufixo +ramal
		packimsi = packimsi + imsi + sufixo +ramal
		contador1 = contador1 + 1

	sizepack = (len(packimei)/2) + (len(packimsi)/2) + 8 + (len(header)/2)
        
	pack = str("%04X" % sizepack) + header + iddbfc + str("%04X" % totalimei) + packimei + str("%04X" % totalimsi) + packimsi
	print pack
	data_hex = binascii.a2b_hex(pack)
	s.send(data_hex)
      
	

def supp_dbfj_config(iddbfj, dbfjbandas, dbfjstatus, dbfjpots):
   bandas = dbfjbandas.split(",")
   check_val_of_band(bandas)
   status = dbfjstatus.split(",")
   check_val_of_status(status)
   potenc = dbfjpots.split(",")
   check_val_of_pots(potenc)

   lenpack    = str("%04X" % (6*len(bandas) + 15 + 1 + 2 + 1))
   n_bandas   = str("%02X" % len(bandas))
   tipo_placa = '02'
   load = ''

   for k in range(len(bandas)):
      load += "%04X" % int(bandas[k], 10) + "%04X" % int(potenc[k], 10)  +  "%04X" % int(status[k], 10)

   print lenpack
   data_hex = binascii.a2b_hex(lenpack + "000267FBE99F404BE99F40011302"  + iddbfj + n_bandas + load)
   s.send(data_hex)
   print 'DBFJ config enviado'




def supp_list_all_car():
   data_hex = binascii.a2b_hex("000F0002684BE99F404BE99F40A31F")
   print "Vai listar todos os carceris "
   s.send(data_hex)

def usage():
   print '@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@'
   print '@ Necessario passar paramentros...    @'
   print '@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@'


#####################################################
#                 CODIGO PRINCIPAL                  #
#####################################################
if len(sys.argv) == 1:
	 print 'Entrando no modo com menu...'
	 menu_script();
	 sys.exit();

try:
	opts, args = getopt.getopt(sys.argv[1:], "", ["help","dtmf=","tipo=","id=","ondbfc=", "offdbfc=", "resetdbfc=", "ondbfj=", "offdbfj=", "resetdbfj=", "dbfjconfig=", "dbfjbandas=", "dbfjstatus=", "dbfjpots=", "listallcarceris", "ipmanager=", "portmanager=", "addimei", "remimei", "atualizalistimei"])

except getopt.GetoptError, err:
	# print help information and exit:
	print str(err) # will print something like "option -a not recognized"
	usage()
   	sys.exit(2)

#Inicializacoes de variaveis 
iddbfc = '0'
iddbfj = '0'
iddbfjconfig = '0'
dbfjbandas = '0'
dbfjstatus = '0'
dbfjpots = '0'
listarcarceris = '0'
ligardbfc = '0'
desligardbfc = '0'
resetardbfc = '0'
ligardbfj = '0'
desligardbfj = '0'
resetardbfj = '0'
ipmanager = '0'
portmanager = '0'
addimei = '0'
addimsi = '0'
remimei = '0'
remimsi = '0'
atualizalistimei = '0'
dtmf='0'
enviar_dtmf='0'

for o, a in opts:
	if o == "--dtmf":
	   dtmf = a
	   check_size_dtmf(dtmf)
   	   enviar_dtmf = '1'

	if o == "--ondbfc":
	   iddbfc = a
	   check_size_id(iddbfc)
   	   ligardbfc = '1'

	elif o in ("--offdbfc"):
	   iddbfc = a
	   check_size_id(iddbfc)
	   desligardbfc = '1'

	elif o in ("--resetdbfc"):
	   iddbfc = a
	   check_size_id(iddbfc)
	   resetardbfc = '1'

	elif o in ("--ondbfj"):
	   iddbfj = a
	   check_size_id(iddbfj)
	   ligardbfj = '1'

	elif o in ("--offdbfj"):
	   iddbfj = a
	   check_size_id(iddbfj)
	   desligardbfj = '1'

	elif o in ("--resetdbfj"):
	   iddbfj = a
	   check_size_id(iddbfj)
	   resetardbfj = '1'

	elif o in ("--dbfjconfig"):
	   iddbfjconfig = a
	   check_size_id(iddbfjconfig)

	elif o in ("--dbfjbandas"):
	   dbfjbandas = a

	elif o in ("--dbfjstatus"):
	   dbfjstatus = a

	elif o in ("--dbfjpots"):
	   dbfjpots = a

	elif o in ("--listallcarceris"):
	   listarcarceris = '1'

	elif o in ("--ipmanager"):
	   ipmanager = a

	elif o in ("--portmanager"):
	   portmanager = a

	elif o in ("--addimei"):
	   addimei = '1';
	
	elif o in ("--addimsi"):
	   addimsi = '1';

	elif o in ("--remimei"):
	   remimei = '1';
	
	elif o in ("--remimsi"):
	   remimsi = '1';

	elif o in ("--atualizalistimei"):
	   atualizalistimei = '1'

	elif o in ("--help"):
	   usage()
	   sys.exit()
#else:
#	   assert False, "Opcao invalida!"

if(ipmanager == '0' or portmanager == '0'):
   print '**************************** ERRO **************************************'
   print 'Necessario passar IP e Porta do Manager. [--ipmanager    --portmanager]'
   print '************************************************************************'
   sys.exit()

s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((ipmanager, int(portmanager)))

if(enviar_dtmf == '1'):
   supp_send_dtmf(dtmf)

if(addimei == '1'):
   supp_add_imei()

if(addimsi == '1'):
   supp_add_imsi()

if(remimei == '1'):
   supp_rem_imei()

if(remimsi == '1'):
   supp_rem_imsi()

if(listarcarceris == '1'):
   supp_list_all_car()

if(ligardbfc == '1'):
   supp_on_dbfc(iddbfc)

if(desligardbfc == '1'):
   supp_off_dbfc(iddbfc)

if(resetardbfc == '1'):
   supp_off_dbfc(iddbfc)
   time.sleep(10)
   supp_on_dbfc(iddbfc)

if(ligardbfj == '1'):
   supp_on_dbfj(iddbfj)

if(desligardbfj == '1'):
   supp_off_dbfj(iddbfj)

if(resetardbfj == '1'):
   supp_off_dbfj(iddbfj)
   supp_on_dbfj(iddbfj)

if(atualizalistimei == '1'):
   atualiza_lista_imei()

if (iddbfjconfig != '0' and dbfjbandas != '0' and dbfjstatus != '0' and dbfjpots != '0'):
   supp_dbfj_config(iddbfjconfig, dbfjbandas, dbfjstatus, dbfjpots)

s.close()
