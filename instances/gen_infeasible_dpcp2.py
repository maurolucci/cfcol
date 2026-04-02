# -*- coding: utf-8 -*-
# Crear instancias infactibles de DPCP
# A partir de replicar antiwebs AW(n,p)
# El número de vértices de la instancia DPCP es n*(r-1)
# donde r= ceil(n/p) es el número cromático del AW(n,p)

# Parámetros del AW(n,p)
n = 20  # número de nodos
p = 2  # no-vecinos consecutivos, p=1 es la clique, p=2 es el anticiclo
# número cromático del AW(n,p):
r = n // p if n % p ==0 else (n // p) + 1

print(f'AW({n},{p}), χ={r}')

# Construcción del grafo del AW(n,p)
V = list(range(n))
G = [[] for i in V]
for i in V:
    for j in V:
        if j<=i:
            continue
        if (j-i) >= p and (j-i)<= (n-p):
            G[i].append(j)

# Construcción del grafo G2 para DPCP
Vp = [(v,k) for v in V for k in range(r-1)]
n2 = len(Vp)
print(f'Número de nodos en Gp: {n2}')
V2dict = {i : Vp[i] for i in range(n2)}
V2 = list(V2dict.keys())
G2 = [[] for i in V2]
for i in V2:
    for j in V2:
        if j<=i:
            continue
        (v1, k1) = V2dict[i]
        (v2, k2) = V2dict[j]
        # Agregar arista ij en Gp si v1==v2 o (k1==k2 y v1,v2 in E) 
        if (v1==v2) or ((k1==k2) and ((v2 in G[v1]) or (v1 in G[v2]))):
            G2[i].append(j)

m2 = sum(len(L) for L in G2)
print(f'Número de aristas en Gp: {m2}')

# Particiones
nP = n
P = [[i for i in V2 if V2dict[i][0]==pi] for pi in V]
nQ = r-1
Q = [[i for i in V2 if V2dict[i][1]==qj] for qj in range(r-1)]
# Verificar que son particiones
for i in V2:
    assert(len([L for L in P if i in L])==1) 
    assert(len([L for L in Q if i in L])==1)

# Escritura de archivos
nombre_base = f'infeasibles/AW({n},{p})'

# # Archivo del grafo antiweb
# archivo_grafo = nombre_base + '.graph'
# f = open(archivo_grafo, "w")
# f.write(f'{n}:{sum(len(L) for L in G)}\n')
# for i in V:
#     for j in G[i]:
#         f.write(f'{i},{j}\n')

# Archivo del grafo DPCP
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




         

