#include "trail.hpp"

Trail::Trail(float trailWidth, int bufferSize) : _trailBuffer(bufferSize) ,_vao(0), _vbo(0), _ibo(0), _trailWidth(trailWidth), _frontIndex(0), _backIndex(0), _score(0)
{

}

Trail::~Trail()
{

}

void Trail::setTrailWidth(float trailWidth)
{
    _trailWidth = trailWidth;
}

void Trail::initGL()
{
    glGenBuffers(1, &_vbo);
    glGenBuffers(1, &_ibo);

    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, (_trailBuffer - 1) * 4 * sizeof(Vertex) , nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (_trailBuffer - 1) * 6 * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);

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


void Trail::reInitGL() {
    glDeleteVertexArrays(1, &_vao);
    glGenVertexArrays(1, &_vao);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
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

//Add a point to the trail.
void Trail::pushBack(const glm::vec3& pointPosition, const glm::vec3& color)
{
    int trailSize = _trailPoints.size();

    //activate this if you don't want to update trail if object isn't moving.
//    if(trailSize != 0 && glm::length2(_trailPoints[trailSize - 1] - pointPosition) < 0.000001f ) //static ?
//        return;

    if(trailSize == 0)
    {
        _trailPoints.push_back( pointPosition );
        return;
    }
    else //if( trailSize  < _trailBuffer )
    {
        if(trailSize == 1)
        {
             _trailPoints.push_back( pointPosition );
        }
        else
        {
            _trailPoints.push_back( _trailPoints[_backIndex] );
            _trailPoints.push_back( pointPosition );
        }

        trailSize = _trailPoints.size();

        glm::vec3 A = _trailPoints[trailSize - 2];
        glm::vec3 B = _trailPoints[trailSize - 1];
        glm::vec3 tangent = glm::normalize( B - A);
        glm::vec3 normalA(-tangent.y, tangent.x, 0);
        glm::vec3 normalB(-tangent.y, tangent.x, 0);


        //take the previous point into account to smooth the shape.
        if(trailSize >= 4)
        {
            glm::vec3 O =_trailPoints[_backIndex];
            glm::vec3 tangentO = glm::normalize( O - _trailPoints[_backIndex - 1]);
            glm::vec3 normalO(-tangentO.y, tangentO.x, 0);

            glm::vec3 smoothNormal = glm::normalize(normalB + normalO);
            normalA = smoothNormal;

            _trailVertex[_backIndex*2 - 1] = ( Vertex( A + normalA * _trailWidth*0.5f , color ) ); //b
            _trailVertex[_backIndex*2] = ( Vertex( A - normalA * _trailWidth*0.5f , color) ); //c
        }


       // +(a)------------------+(b)                            +
       // |   \                 |                               |
       // |       \      (1)    |                               | normal
       // A          \          B       ------tangent------->   |
       // |      (2)     \      |                               |
       // |                  \  |                               |
       // +(c)------------------+(d)

        int id_a = _trailVertex.size(); // id_b = id_a + 1, id_c = id_a + 2, ...

        _trailVertex.push_back( Vertex( A + normalA * _trailWidth*0.5f , color) ); //a
        _trailVertex.push_back( Vertex( B + normalB * _trailWidth*0.5f , color) ); //b

        _trailVertex.push_back( Vertex( B - normalB * _trailWidth*0.5f , color) ); //c
        _trailVertex.push_back( Vertex( A - normalA * _trailWidth*0.5f , color) ); //d


        //triangle (1) :
        _trailIndex.push_back( id_a );
        _trailIndex.push_back( id_a + 1 );
        _trailIndex.push_back( id_a + 2 );

        //triangle (2) :
        _trailIndex.push_back( id_a );
        _trailIndex.push_back( id_a + 2 );
        _trailIndex.push_back( id_a + 3 );

        _backIndex = _trailPoints.size() - 1;

        //synchronizeVbos();
    }

}

void Trail::synchronizeVbos()
{
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, (_trailBuffer - 1) * 4 * sizeof(Vertex) , nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, _trailVertex.size() * sizeof(Vertex), &_trailVertex[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, (_trailBuffer - 1) * 6 * sizeof(uint32_t), nullptr, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, _trailIndex.size() * sizeof(uint32_t), &_trailIndex[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//Delete the first point put in the trail. (FIFO)
void Trail::popFront()
{

    int frontIndex_points = _frontIndex;
    int frontIndex_vertices = _frontIndex*2;
    int frontIndex_indices = _frontIndex*3;


    // swap the 2 front trail point, with the 2 back trail point
    for(int i = 0; i < 2; i++){
        std::swap(_trailPoints[frontIndex_points +  1 - i], _trailPoints[_trailPoints.size() - 1]);
        _trailPoints.pop_back();
    }


    // swap the 4 front vertices, with the 6 back vertices
    for(int i = 0; i < 4; i++){
        std::swap(_trailVertex[frontIndex_vertices +  3 - i], _trailVertex[_trailVertex.size() - 1]);
        _trailVertex.pop_back();
    }

    // swap the 6 front indices, with the 6 back indices
    for(int i = 0; i < 6; i++){
        std::swap(_trailIndex[frontIndex_indices + 5 - i], _trailIndex[_trailIndex.size() - 1]);
        _trailIndex.pop_back();
    }

    //triangle (1) :
    _trailIndex[frontIndex_indices + 0] = ( frontIndex_vertices + 0 );
    _trailIndex[frontIndex_indices + 1] = ( frontIndex_vertices + 1 );
    _trailIndex[frontIndex_indices + 2] = ( frontIndex_vertices + 2 );

    //triangle (2) :
    _trailIndex[frontIndex_indices + 3] = ( frontIndex_vertices + 0 );
    _trailIndex[frontIndex_indices + 4] = ( frontIndex_vertices + 2 );
    _trailIndex[frontIndex_indices + 5] = ( frontIndex_vertices + 3 );

    //synchronizeVbos();

    _frontIndex += 2;
    _backIndex = _frontIndex - 1;
    if(_frontIndex >= _trailPoints.size())
        _frontIndex = 0;

}

void Trail::popFront_naive()
{
    _trailPoints.erase(_trailPoints.begin(), _trailPoints.begin()+2);
    _trailVertex.erase(_trailVertex.begin(), _trailVertex.begin()+4);
    _trailIndex.erase(_trailIndex.begin(), _trailIndex.begin()+6);
    _backIndex = _trailPoints.size() - 1;
}

void Trail::draw()
{
    glBindVertexArray(_vao);
    int indexCount = getIndexCount();
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Trail::clearGL()
{
    glDeleteBuffers(1, &_vbo);
    glDeleteVertexArrays(1, &_vao);
}

void Trail::update()
{
    //for(int i = 0; i < _trailPoints.size(); i++)
    //    std::cout<<"point["<<i<<"] = "<<_trailPoints[i].x<<", "<<_trailPoints[i].y<<", "<<_trailPoints[i].z<<std::endl;

    if(_trailPoints.size() > _trailBuffer)
    {
        popFront();
    }
}

int Trail::getIndexCount()
{
    return _trailIndex.size();
}

bool Trail::isCollide(const Trail &other, float radiusCollider) const {
    if(_trailPoints.empty())
        return false;

    glm::vec3 lastPoint = _trailPoints[_trailPoints.size()-1];
    for(auto& p: other._trailPoints){
        if(glm::distance2(glm::vec2(p.x, p.y), glm::vec2(lastPoint.x, lastPoint.y)) < radiusCollider)
            return true;
    }
    return false;
}

int &Trail::score() {
    return _score;
}

void Trail::setScore(int score) {
    _score = score;
}
