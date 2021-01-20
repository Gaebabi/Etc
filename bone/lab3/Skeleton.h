#pragma once
#include <GL/gl3w.h>
#include <glm\glm.hpp>
#include "Loader.h"
#include <vector>
#include <map>

using namespace std;

class Skeleton {
public:
	vector<glm::vec3> skeleton;
	vector<unsigned int> indices;

	Skeleton(std::map<unsigned int, glm::vec3> skeleton_map) {
		initSkeleton(skeleton_map);
		setupSkeleton();
	}

	//render the Skeleton
	void Draw(ShaderProgram * shader)
	{
		shader->use();
		glBindVertexArray(skeletonVAO);
		//glDrawArrays(GL_POINTS, 0, skeleton.size());
		glDrawElements(GL_POINTS, indices.size(), GL_UNSIGNED_INT, 0);

		glBindVertexArray(0);
	}

private:
	unsigned int skeletonVAO, VBO, EBO;

	void initSkeleton(std::map<unsigned int, glm::vec3> skeleton_map)
	{
		/*skeleton.resize(skeleton_map.size());
		indices.resize(skeleton_map.size());*/
		for (auto it = skeleton_map.cbegin(); it != skeleton_map.cend(); ++it)
		{
			indices.push_back(it->first);
			skeleton.push_back(it->second);
		}
	}

	void setupSkeleton()
	{
		glGenVertexArrays(1, &skeletonVAO);
		glBindVertexArray(skeletonVAO);

		glGenBuffers(1, &EBO);
		glGenBuffers(1, &VBO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, skeleton.size() * sizeof(glm::vec3), &skeleton[0], GL_STATIC_DRAW);
		
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
};