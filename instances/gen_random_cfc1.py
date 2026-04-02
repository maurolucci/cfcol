# Crear instancias aleatorias de CFC y convertirlas a DPCP
# Se definen instancias de CFC a partir de hipergrafos aleatorios
# Para cada hipergrafo, se especifica el número de vértices n, el número de hiperaristas m,
# y la probabilidad p de que un vértice pertenezca a una hiperarista.
# A partir de la instancia CFC, se genera la instancia DPCP correspondiente.

# Parámetros de la línea de comandos:
# n : número de vértices del hipergrafo
# p : probabilidad de que un vértice pertenezca a una hiperarista
# m : número de hiperaristas
# path : ruta hacia la carpeta de salida
# Ejemplo de uso:
# python3 aleatorio_cfc1.py 50 100 0.2 ./instancias


import argparse
import random   

# Función para generar una instancia aleatoria de CFC
def random_cfc_1(n, m, p):
    V = list(range(n))
    EE = []
    for k in range(m):
        e = []
        for i in V:
            if random.random() < p:
                e.append(i)
        if len(e) == 0:
            e.append(random.choice(V))
        EE.append(e)
    return V, EE

parser = argparse.ArgumentParser(description="Procesa archivos PCP.")
parser.add_argument("n", help="Número de vértices del hipergrafo", type=int)
parser.add_argument("m", help="Número de aristas del hipergrafo", type=int)
parser.add_argument("p", help="Probabilidad de que un vértice pertenezca a una hiperarista", type=float)
parser.add_argument("path", help="Ruta hacia la carpeta de salida")
args = parser.parse_args()

n = args.n
m = args.m
p = args.p
path = args.path
nombre_base = f'{path}/n{n}_m{m}_p{p}'
archivo = nombre_base + ".cfc"

# Generación de la instancia CFC
V, EE = random_cfc_1(n, m, p)

# Escritura de la instancia CFC en archivo
f = open(archivo, "w")
f.write(f'{n} {m}\n')
for e in EE:
    L = [str(i) for i in e]
    s = f'{len(e)} ' + ' '.join(L) + '\n'
    f.write(s)
print(f'Archivo {archivo} escrito con éxito.')  

# Creación de la instancia de DPCP

# Nodos
# Los nodos son pares (E,v) donde E es una hiperarista y v es un vértice en E
# Representamos los nodos como enteros desde 0 en la lista V2
# Almacenamos el par (E,v) en un diccionario V2dict 
V2dict = {}
for e in EE:
    for v in e:
        V2dict[len(V2dict)] = (EE.index(e), v)  # (E,v)
V2 = list(V2dict.keys())
n2 = len(V2)    

# Aristas
# Hay una arista entre i1=(E1,v1) y i2=(E2,v2) si i1<i2, v1!=v2, y ((v1 in E2) or (v2 in E1))
G2 = {i:[] for i in V2}
m2 = 0
for i1 in V2:
    E1, v1 = V2dict[i1]
    for i2 in V2:
        if i2 <= i1:
            continue
        E2, v2 = V2dict[i2]
        if v1 != v2 and (v1 in EE[E2] or v2 in EE[E1]):
            G2[i1].append(i2)
            m2 += 1


# Particiones
nP = m
P = [[i for i in V2 if V2dict[i][0]==pi] for pi in range(nP)]
nQ = n
Q = [[i for i in V2 if V2dict[i][1]==qj] for qj in range(nQ)]
# Verificar que son particiones
for i in V2:
    assert(len([L for L in P if i in L])==1) 
    assert(len([L for L in Q if i in L])==1) 
    
# Exportar archivos
# Archivo del grafo
nombre_base += '.dpcp'
archivo_grafo = nombre_base + '.graph'
f = open(archivo_grafo, "w")
f.write(f'{n2}:{m2}\n')
for i in V2:
    for j in G2[i]:
        f.write(f'{i} {j}\n')

# Archivo de claves de nodos
archivo_diccionario = nombre_base + '.dict'
f = open(archivo_diccionario, "w")
f.write(f'{n2}:{nP}:{nQ}\n')
for i in V2:
    f.write(f'{i} {V2dict[i][0]} {V2dict[i][1]}\n')

# Archivo de primera partición
archivo_partP = nombre_base + '.partP'
f = open(archivo_partP, "w")
f.write(f'{n2}:{nP}\n')
for pi in range(len(P)):
    L = [str(i) for i in P[pi]]
    s = f'{pi} ' + f'{len(P[pi])} ' + ' '.join(L) + '\n'
    f.write(s)

# Archivo de segunda partición
archivo_partQ = nombre_base + '.partQ'
f = open(archivo_partQ, "w")
f.write(f'{n2}:{nQ}\n')
for qj in range(len(Q)):
    L = [str(i) for i in Q[qj]]
    s = f'{qj} ' + f'{len(Q[qj])} ' + ' '.join(L) + '\n'
    f.write(s)

print(f'Archivos {archivo_grafo}, {archivo_diccionario}, {archivo_partP} y {archivo_partQ} escritos con éxito.')