#include "Model.h"

extern vector<model> models; //Todos los modelos irán en este vector
extern CGLSLProgram glslProgram; //Programa de shaders

//Crea los VBOS con vértice, normal y textura
void model::initVBO(int size) {
	glGenVertexArrays(1, &(models[models.size() - 1].vao));
	glGenBuffers(1, &(models[models.size() - 1].vbo));
	glBindVertexArray((models[models.size() - 1].vao));
	glBindBuffer(GL_ARRAY_BUFFER, models[models.size() - 1].vbo);
	glBufferData(GL_ARRAY_BUFFER, models[models.size() - 1].vertices.size() * sizeof(float) + models[models.size() - 1].coord_texturas.size() * sizeof(float) + models[models.size() - 1].normales_vertice_fin.size() * sizeof(float), NULL, GL_STATIC_DRAW);
	//Guardo Vertices en el VBO
	glBufferSubData(GL_ARRAY_BUFFER,
		0,
		models[models.size() - 1].vertices.size() * sizeof(float),
		models[models.size() - 1].vertices.data());
	glBufferSubData(GL_ARRAY_BUFFER,
		models[models.size() - 1].vertices.size() * sizeof(float),
		models[models.size() - 1].coord_texturas.size() * sizeof(float),
		models[models.size() - 1].coord_texturas.data());

	glBufferSubData(GL_ARRAY_BUFFER,
		models[models.size() - 1].vertices.size() * sizeof(float) + models[models.size() - 1].coord_texturas.size() * sizeof(float),
		models[models.size() - 1].normales_vertice_fin.size() * sizeof(float),
		models[models.size() - 1].normales_vertice_fin.data());
	glBindVertexArray(0);
}

//Para separar un string
vector<string> split(const string &s, char delim) {
	stringstream ss(s);
	string item;
	vector<string> tokens;
	while (getline(ss, item, delim)) {
		tokens.push_back(item);
	}
	return tokens;
}

//Lector de OBJ (con normales, textura y posición de modelos en el mapa)
void model::read_obj(char *filename) {
	vector<vertice> auxVertices_1, auxVertices_2;
	vector<vertice> coord_texturasAux;
	char line[4096] = ""; //Leo 4096 char en una línea
	ifstream file;
	bool centrado = true;
	file.open(filename);
	string comprobacion;
	double x, y, z, minX = INT_MAX, minY = INT_MAX, minZ = INT_MAX, maxX = INT_MIN, maxY = INT_MIN, maxZ = INT_MIN;
	vertice v_secundario, normal;
	model m;
	vector<face> faces;
	while (!file.eof()) { //Hasta terminar el archivo
		file >> comprobacion; // Leo el primer string (para ver si es vn, vt, v o f)
		if (comprobacion == "v") {
			//Como es v leo los x y z, agrego al vector y sumo un vértice
			file >> x >> y >> z;
			if (x>maxX) maxX = x; if (y>maxY) maxY = y; if (z>maxZ) maxZ = z;
			if (x<minX) minX = x; if (y<minY) minY = y; if (z<minZ) minZ = z;
			v_secundario.x = x;
			v_secundario.y = y;
			v_secundario.z = z;
			auxVertices_1.push_back(v_secundario);
		}
		else if (comprobacion == "vt") {
			//Obtengo las coordenadas de texturas
			file >> x >> y;
			vertice a;
			a.x = x;
			a.y = y;
			coord_texturasAux.push_back(a);
		}
		else if (comprobacion == "f") { //Si es f leo la cara                     
										//Obtengo la cara
			face cara;
			int nFacesAux = 0;
			file.getline(line, 4096); //Obtengo toda la línea (sin el "f")
			vector<string> indices_vertices;
			string f_auxiliar = "";
			string auxiliar = line;
			auxiliar.erase(auxiliar.find_last_not_of(" ") + 1); //Borro espacios al final de la línea
			stringstream ss(auxiliar);
			while (!ss.eof()) { //Recorro la línea que agarré hasta el final
				ss >> f_auxiliar; //Leo strings
				if (f_auxiliar != "\0") {
					indices_vertices = split(f_auxiliar, '/'); //Separo el string por "/"
					nFacesAux++;
					const char * c = indices_vertices[0].c_str(); //Siempre el [0] sera el vértice, lo transformo a int
					int real_index = atoi(c) - 1;
					cara.f.push_back(real_index);
					c = indices_vertices[1].c_str(); //Siempre el [1] sera la textura, la transformo a int
					real_index = atoi(c) - 1;
					cara.t.push_back(real_index);
					auxiliar = f_auxiliar;
				}

			}
			//Terminé esa cara, pusheo al vector de caras
			if (nFacesAux != 0) {
				cara.n_vertex = nFacesAux;
				faces.push_back(cara);
			}

		}
		else if ((comprobacion[0] != 'v' && comprobacion[0] != 'f')) {
			//Si es vn salto la línea
			file.getline(line, 4096);
		}
	}

	//Saco los vértices centrales del modelo
	double cx = (maxX + minX) / 2.0; double cy = (maxY + minY) / 2.0; double cz = (maxZ + minZ) / 2.0;
	vertice v, nor;
	double maxXYZ = max(maxX, max(maxY, maxZ));
	minX = INT_MAX, minY = INT_MAX, minZ = INT_MAX, maxX = INT_MIN, maxY = INT_MIN, maxZ = INT_MIN;

	vector<int> totales;
	vector<vertice> normales_pavo;

	for (int i = 0; i < auxVertices_1.size(); i++) {

		v.x = (auxVertices_1[i].x - cx) / maxXYZ;
		v.y = (auxVertices_1[i].y - cy) / maxXYZ;
		v.z = (auxVertices_1[i].z - cz) / maxXYZ;
		if (v.x>maxX) maxX = v.x; if (v.y>maxY) maxY = v.y; if (v.z>maxZ) maxZ = v.z;
		if (v.x<minX) minX = v.x; if (v.y<minY) minY = v.y; if (v.z<minZ) minZ = v.z;

		auxVertices_2.push_back(v);

		nor.x = 0;
		nor.y = 0;
		nor.z = 0;
		normales_pavo.push_back(nor);

		totales.push_back(0);
	}

	for (int i = 0; i < faces.size(); i++) {
		int v1, v2, v3, t1, t2, t3, aux = 1, k = 0, aux2 = 1;
		vertice u, v;

		while (faces[i].n_vertex - 1 > aux) {
			v1 = faces[i].f[k];
			v2 = faces[i].f[aux];
			v3 = faces[i].f[++aux];

			m.vertices.push_back(auxVertices_2[v1].x);
			m.vertices.push_back(auxVertices_2[v1].y);
			m.vertices.push_back(auxVertices_2[v1].z);

			m.vertices.push_back(auxVertices_2[v2].x);
			m.vertices.push_back(auxVertices_2[v2].y);
			m.vertices.push_back(auxVertices_2[v2].z);

			m.vertices.push_back(auxVertices_2[v3].x);
			m.vertices.push_back(auxVertices_2[v3].y);
			m.vertices.push_back(auxVertices_2[v3].z);

			t1 = faces[i].t[k];
			t2 = faces[i].t[aux2];
			t3 = faces[i].t[++aux2];

			m.coord_texturas.push_back(coord_texturasAux[t1].x);
			m.coord_texturas.push_back(coord_texturasAux[t1].y);
			m.coord_texturas.push_back(coord_texturasAux[t2].x);
			m.coord_texturas.push_back(coord_texturasAux[t2].y);
			m.coord_texturas.push_back(coord_texturasAux[t3].x);
			m.coord_texturas.push_back(coord_texturasAux[t3].y);

			//Saco normales
			u.x = auxVertices_2[v2].x - auxVertices_2[v1].x;
			u.y = auxVertices_2[v2].y - auxVertices_2[v1].y;
			u.z = auxVertices_2[v2].z - auxVertices_2[v1].z;
			v.x = auxVertices_2[v3].x - auxVertices_2[v1].x;
			v.y = auxVertices_2[v3].y - auxVertices_2[v1].y;
			v.z = auxVertices_2[v3].z - auxVertices_2[v1].z;
			normal.x = u.y*v.z - u.z*v.y;
			normal.y = u.z*v.x - u.x*v.z;
			normal.z = u.x*v.y - u.y*v.x;

			//Guardo suma de normales por vertice
			normales_pavo[v1].x += normal.x;
			normales_pavo[v1].y += normal.y;
			normales_pavo[v1].z += normal.z;

			normales_pavo[v2].x += normal.x;
			normales_pavo[v2].y += normal.y;
			normales_pavo[v2].z += normal.z;

			normales_pavo[v3].x += normal.x;
			normales_pavo[v3].y += normal.y;
			normales_pavo[v3].z += normal.z;

			//Guardo total de la suma de normales para dividir
			totales[v1] += 1;
			totales[v2] += 1;
			totales[v3] += 1;

		}
	}

	for (int i = 0; i < faces.size(); i++) {
		int v1, v2, v3, aux = 1, k = 0;
		vertice u, v, final;

		while (faces[i].n_vertex - 1 > aux) {
			v1 = faces[i].f[k];
			v2 = faces[i].f[aux];
			v3 = faces[i].f[++aux];
			final.x = (normales_pavo[v1].x / (float)totales[v1]);
			final.y = (normales_pavo[v1].y / (float)totales[v1]);
			final.z = (normales_pavo[v1].z / (float)totales[v1]);

			m.normales_vertice_fin.push_back(final.x); m.normales_vertice_fin.push_back(final.y); m.normales_vertice_fin.push_back(final.z);

			final.x = (normales_pavo[v2].x / (float)totales[v2]);
			final.y = (normales_pavo[v2].y / (float)totales[v2]);
			final.z = (normales_pavo[v2].z / (float)totales[v2]);

			m.normales_vertice_fin.push_back(final.x); m.normales_vertice_fin.push_back(final.y); m.normales_vertice_fin.push_back(final.z);

			final.x = (normales_pavo[v3].x / (float)totales[v3]);
			final.y = (normales_pavo[v3].y / (float)totales[v3]);
			final.z = (normales_pavo[v3].z / (float)totales[v3]);

			m.normales_vertice_fin.push_back(final.x); m.normales_vertice_fin.push_back(final.y); m.normales_vertice_fin.push_back(final.z);
		}
	}
	m.minX = minX;
	m.maxX = maxX;
	m.minY = minY;
	m.maxY = maxY;
	m.minZ = minZ;
	m.maxZ = maxZ;

	models.push_back(m); //Pusheo el modelo a vector de modelos
	if (models.size() - 1 == 0) {	//Plano
		models[models.size() - 1].scaleT = 5.00;
		models[models.size() - 1].ejeX = 1.51;
		models[models.size() - 1].ejeY = 0.26;
		models[models.size() - 1].ejeZ = -1.33;
	}
	file.close();
	initVBO(models.size());
}

//Para la rotación (autorotación)
void model::setQuat(const float *eje, float angulo, float *quat) {
	float sina2, norm;
	sina2 = (float)sin(0.5f * angulo);
	norm = (float)sqrt(eje[0] * eje[0] + eje[1] * eje[1] + eje[2] * eje[2]);
	quat[0] = sina2 * eje[0] / norm;
	quat[1] = sina2 * eje[1] / norm;
	quat[2] = sina2 * eje[2] / norm;
	quat[3] = (float)cos(0.5f * angulo);
}

//Para la rotación (autorotación)
void model::multiplicarQuat(const float *q1, const float *q2, float *qout) {
	float qr[4];
	qr[0] = q1[3] * q2[0] + q1[0] * q2[3] + q1[1] * q2[2] - q1[2] * q2[1];
	qr[1] = q1[3] * q2[1] + q1[1] * q2[3] + q1[2] * q2[0] - q1[0] * q2[2];
	qr[2] = q1[3] * q2[2] + q1[2] * q2[3] + q1[0] * q2[1] - q1[1] * q2[0];
	qr[3] = q1[3] * q2[3] - (q1[0] * q2[0] + q1[1] * q2[1] + q1[2] * q2[2]);
	qout[0] = qr[0]; qout[1] = qr[1]; qout[2] = qr[2]; qout[3] = qr[3];
}

//Para crear la matriz de escalamiento
glm::mat4 model::scale_en_matriz(float scale_tx) {
	glm::mat4 scaleMatrix = glm::mat4(glm::vec4(scale_tx, 0.0, 0.0, 0.0), glm::vec4(0.0, scale_tx, 0.0, 0.0), glm::vec4(0.0, 0.0, scale_tx, 0.0), glm::vec4(0.0, 0.0, 0.0, 1)); //Creo matriz de escalamiento
	return scaleMatrix;
}

//Para crear la matriz de translate
glm::mat4 model::translate_en_matriz(float translate_tx, float translate_ty, float translate_tz) {
	glm::mat4 translateMatrix = glm::mat4(glm::vec4(1.0, 0.0, 0.0, 0.0), glm::vec4(0.0, 1.0, 0.0, 0.0), glm::vec4(0.0, 0.0, 1.0, 0.0), glm::vec4(translate_tx, translate_ty, translate_tz, 1)); //Creo matriz de translate
	return translateMatrix;
}

//Para crear la matriz de rotación
glm::mat4 model::rotacion_en_matriz(float rotacion_tx, float rotacion_ty, float rotacion_tz, float rotacion_ta) {
	glm::mat4 rotateMatrix;
	//Creo matriz de rotación usando los quat
	rotateMatrix[0][0] = 1.0 - 2.0 * (rotacion_ty * rotacion_ty + rotacion_tz * rotacion_tz);
	rotateMatrix[0][1] = 2.0 * (rotacion_tx * rotacion_ty + rotacion_tz * rotacion_ta);
	rotateMatrix[0][2] = 2.0 * (rotacion_tx * rotacion_tz - rotacion_ty * rotacion_ta);
	rotateMatrix[0][3] = 0.0;
	rotateMatrix[1][0] = 2.0 * (rotacion_tx * rotacion_ty - rotacion_tz * rotacion_ta);
	rotateMatrix[1][1] = 1.0 - 2.0 * (rotacion_tx * rotacion_tx + rotacion_tz * rotacion_tz);
	rotateMatrix[1][2] = 2.0 * (rotacion_ty * rotacion_tz + rotacion_tx * rotacion_ta);
	rotateMatrix[1][3] = 0.0;
	rotateMatrix[2][0] = 2.0 * (rotacion_tx * rotacion_tz + rotacion_ty * rotacion_ta);
	rotateMatrix[2][1] = 2.0 * (rotacion_ty * rotacion_tz - rotacion_tx * rotacion_ta);
	rotateMatrix[2][2] = 1.0 - 2.0 * (rotacion_tx * rotacion_tx + rotacion_ty * rotacion_ty);
	rotateMatrix[2][3] = 0.0;
	rotateMatrix[3][0] = 0.0;
	rotateMatrix[3][1] = 0.0;
	rotateMatrix[3][2] = 0.0;
	rotateMatrix[3][3] = 1.0;
	return rotateMatrix;
}