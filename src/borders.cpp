#include "borders.hpp"

Borders::Borders(const std::vector<cv::Vec3d> corners, const glm::vec3& color)
{
    for(int i = 0 ;i < corners.size(); i++)
    {
        _borderVertex.push_back( Vertex( glm::vec3(corners[i][0],corners[i][1],corners[i][2]), color) );
    }

    initGL();
}

Borders::Borders(const std::vector<glm::vec3> corners, const glm::vec3& color)
{
    for(int i = 0 ;i < corners.size(); i++)
    {
        _borderVertex.push_back( Vertex( corners[i], color) );
    }

    initGL();
}


void Borders::draw()
{
    glLineWidth(10.f);

    glBindVertexArray(_vao);
    glDrawArrays(GL_LINE_LOOP, 0, _borderVertex.size());
    glBindVertexArray(0);
}


void Borders::initGL()
{
    glGenBuffers(1, &_vbo);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, _borderVertex.size() * sizeof(Vertex) , &_borderVertex[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

        glBindBuffer(GL_ARRAY_BUFFER, _vbo);
            glEnableVertexAttribArray(0); //position
            glEnableVertexAttribArray(1); //color
            glEnableVertexAttribArray(2); //UV coords
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*) offsetof(Vertex, position));
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*) offsetof(Vertex, color));
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const GLvoid*) offsetof(Vertex, uvCoord));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}
