const float PI = 3.1415926538;
vec4 debugColor = vec4(0.5, 0, 0.8, 1);
const int HIT_NOTHING = 0;
const int HIT_SOLID = 1;
const int HIT_DEBUG = -1;
uniform int maxBounces;
uniform vec2 windowSize;

float hypDot(vec4 v1, vec4 v2){
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z - v1.w * v2.w;
}

float hypLengthSq(vec4 v) {
    return abs(hypDot(v, v));
}

float hypLength(vec4 v) {
    return sqrt(hypLengthSq(v));
}

vec4 hypNormalize(vec4 v) {
    return v / hypLength(v);
}
                                                                                                                        
struct Isometry{
    mat4 matrix;
};
   
const Isometry IDENTITY = Isometry(mat4(1.));                          
   
Isometry reduceError(Isometry isom){
    return isom;
}
   
Isometry multiply(Isometry isom1, Isometry isom2) {
    Isometry res = Isometry(isom1.matrix * isom2.matrix);
    return reduceError(res);
}
   
Isometry geomInverse(Isometry isom) {
    mat4 inv = inverse(isom.matrix);
    Isometry res = Isometry(inv);
    return reduceError(res);

}
                                                                                                                        
struct Point{
    vec4 coords;
};

const Point ORIGIN = Point(vec4(0, 0, 0, 1));                               
   
Point reduceError(Point p){
    vec4 coords = hypNormalize(p.coords);
    return Point(coords);
}
   
Point applyIsometry(Isometry isom, Point p) {
    vec4 coords = isom.matrix * p.coords;
    Point res= Point(coords);
    return reduceError(res);
}

Isometry makeTranslation(Point p) {
    mat4 matrix = mat4(1.);
    vec3 u = p.coords.xyz;
    float c1 = length(u);

    if (c1 == 0.){
        return Isometry(matrix);
    }

    float c2 = p.coords.w-1.;
    u = normalize(u);

    mat4 m = mat4(
    0., 0., 0., u.x,
    0., 0., 0., u.y,
    0., 0., 0., u.z,
    u.x, u.y, u.z, 0.
    );

    matrix = matrix + c1 * m + c2 * m * m;
    Isometry res = Isometry(matrix);
    return reduceError(res);
}
   
Isometry makeInvTranslation(Point p) {
    Isometry isom = makeTranslation(p);
    return geomInverse(isom);
}
                                                                                                                        
struct Vector{
    Point pos;                         
    vec4 dir;
                                                
};

Vector zeroVector(Point pos){
    return Vector(pos, vec4(0));
}
   
Vector reduceError(Vector v){
    v.pos = reduceError(v.pos);
    v.dir = v.dir + hypDot(v.pos.coords, v.dir) * v.pos.coords;
    return v;
}
   
Vector add(Vector v1, Vector v2){
    return Vector(v1.pos, v1.dir + v2.dir);
}
   
Vector sub(Vector v1, Vector v2){
    return Vector(v1.pos, v1.dir - v2.dir);
}
   
Vector multiplyScalar(float s, Vector v){
    return Vector(v.pos, s * v.dir);
}
   
float geomDot(Vector v1, Vector v2) {
    mat4 g = mat4(
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, -1
    );

    return dot(v1.dir, g*v2.dir);
}
   
Vector applyIsometry(Isometry isom, Vector v) {
    vec4 coords = isom.matrix * v.pos.coords;
    Point pos = Point(coords);
    Vector res = Vector(pos, isom.matrix * v.dir);
    return reduceError(res);
}
   
Vector applyFacing(mat4 m, Vector v) {
    return Vector(v.pos, m * v.dir);
}

void initFlow(Vector v){
}

Vector geomMix(Vector v1, Vector v2, float a){
    return add(multiplyScalar(1.-a, v1), multiplyScalar(a, v2));
}
   
Vector negate(Vector v) {
    return multiplyScalar(-1., v);
}
   
float geomLength(Vector v){
    return sqrt(geomDot(v, v));
}
   
Vector geomNormalize(Vector v){
    float a = geomLength(v);
    return multiplyScalar(1./a, v);
}
   
float cosAngle(Vector v1, Vector v2){
    float a1 = geomLength(v1);
    float a2 = geomLength(v2);
    return geomDot(v1, v2)/ (a1 * a2);
}
   
Vector geomReflect(Vector v, Vector n){
    return sub(v, multiplyScalar(2. * geomDot(v, n), n));
}
   
Vector geomRefract(Vector v, Vector n, float r){
    float cosTheta1 = -geomDot(n, v);
    float sinTheta2Sq = r * r * (1. - cosTheta1 * cosTheta1);

    if (sinTheta2Sq > 1.){
               
        return zeroVector(v.pos);
    }
                                                                 
    float cosTheta2 = sqrt(1. - sinTheta2Sq);
    float aux = r * cosTheta1 - cosTheta2;
    return add(multiplyScalar(r, v), multiplyScalar(aux, n));
}
   
Isometry makeTranslation(Vector v) {
    return makeTranslation(v.pos);
}
   
Isometry makeInvTranslation(Vector v) {
    return makeInvTranslation(v.pos);
}




   
                               
                                                                       
                                     
                                                      
                                                                                             
                                     
   
void frame(Point p, out Vector[3] f){
    float x = p.coords.x;
    float y = p.coords.y;
    float z = p.coords.z;
    float w = p.coords.w;

    vec4 dir0 = vec4(w, 0, 0, x);
    vec4 dir1 = vec4(0, w, 0, y);
    vec4 dir2 = vec4(0, 0, w, z);

    float l0 = sqrt(abs(w * w - x * x));
    float l1 = sqrt(abs(w * w - y * y));
    float l2 = sqrt(abs(w * w - z * z));

    f[0] = Vector(p, dir0 / l0);
    f[1] = Vector(p, dir1 / l1);
    f[2] = Vector(p, dir2 / l2);
}


   
                                           
                                                                       
                                     
                                                      
   
void orthoFrame(Point p, out Vector[3] f){
    float x = p.coords.x;
    float y = p.coords.y;
    float z = p.coords.z;
    float w = p.coords.w;

    float den = 1. + w;
    vec4 dir0 = (1. / den) * vec4(x * x + w + 1., x * y, x * z, x * den);
    vec4 dir1 = (1. / den) * vec4(x * y, y * y + w + 1., y * z, y * den);
    vec4 dir2 = (1. / den) * vec4(x * z, y * z, z * z + w + 1., z * den);

    f[0] = Vector(p, dir0);
    f[1] = Vector(p, dir1);
    f[2] = Vector(p, dir2);
}


   
                                                                                         
                              
                                                                                              
   
Point smallShift(Point p, vec3 dp){
    Vector[3] f;
    frame(p, f);
    vec4 coords=p.coords+dp[0]*f[0].dir+dp[1]*f[1].dir+dp[2]*f[2].dir;

    Point res=Point(coords);
    return reduceError(res);

}


Vector smallShift(Vector v, vec3 dp){
    Point pos = smallShift(v.pos, dp);
    return Vector(pos, v.dir);
}


   
                                  
                                                 
   
Vector flow(Vector v, float t){
    vec4 coords = cosh(t) * v.pos.coords + sinh(t) * v.dir;
    Point pos = Point(coords);
    vec4 dir = sinh(t) * v.pos.coords + cosh(t) * v.dir;
    Vector res = Vector(pos, dir);
    res = reduceError(res);
    return geomNormalize(res);
}




                                                                                                                        
                                                                                                                        
  
                                                               
  
                                                                                                                        
                                                                                                                        

   
                                                                         
               
   
Vector createVector(Point p, vec3 coords, Vector[3] frame){
    Vector c0 = multiplyScalar(coords[0], frame[0]);
    Vector c1 = multiplyScalar(coords[1], frame[1]);
    Vector c2 = multiplyScalar(coords[2], frame[2]);
    return add(c0, add(c1, c2));
}



   
                                                                                          
               
   
Vector createVector(Point p, vec3 coords){
    Vector[3] f;
    frame(p, f);
    return createVector(p, coords, f);
}

   
                                                                                          
               
   
Vector createVectorOrtho(Point p, vec3 coords){
    Vector[3] f;
    orthoFrame(p, f);
    return createVector(p, coords, f);
}


                                                                                                                        
  
                   
                                                             
                                                                       
  
                                                                                                                        

struct Position {
    Isometry boost;
    mat4 facing;
};


   
                                        
                          
                                           
   
Vector applyPosition(Position p, Vector v){
    Vector res = applyFacing(p.facing, v);
    return applyIsometry(p.boost, res);
}



                                                                                                                        
                                                                                                                        
  
                              
  
                                                                                                                        
                                                                                                                        

struct VectorData {
    float lastFlowDist;                                                                    
    float lastBounceDist;                                                           
    float totalDist;                                                  
    bool isTeleported;                                             
    int iMarch;                                                        
    int iBounce;                                             
    bool stop;                              
    vec4 pixel;                                                                   
    vec4 leftToComputeColor;                                                                      
};



                                                                                                                        
                                                                                                                        
  
                     
  
                                                                                                                        
                                                                                                                        

                                                                                                                        
          
                                    
                                                                                                                        

struct GroupElement {
    Isometry isom;                                 
};

const GroupElement GROUP_IDENTITY = GroupElement(IDENTITY);

GroupElement multiply(GroupElement elt1, GroupElement elt2){
    return GroupElement(multiply(elt1.isom, elt2.isom));
}

                                              
                                                 
   

Isometry toIsometry(GroupElement elt) {
    return elt.isom;
}



   
              
                                
   
Point applyIsometry(GroupElement elt, Point p){
    return applyIsometry(toIsometry(elt), p);
}

Point applyGroupElement(GroupElement elt, Point p){
    return applyIsometry(toIsometry(elt), p);
}

   
              
                                
   
Vector applyIsometry(GroupElement elt, Vector v){
    return applyIsometry(toIsometry(elt), v);
}

Vector applyGroupElement(GroupElement elt, Vector v){
    return applyIsometry(toIsometry(elt), v);
}






                                                                                                                        
  
                      
                                                        
                                                                     
                        
                                                                                 
                                                                                 
                                                                                   
                                                                                    
                                                                       
                                                                                                                        

struct RelPosition {
    Position local;
    GroupElement cellBoost;
    GroupElement invCellBoost;
};


                                                                                                                        
  
                    
                                  
                                                                       
                      
                                                                                 
                                                                                 
                                                                            
                                                                                    
                                                                                                                        

struct RelVector {
    Vector local;
    GroupElement cellBoost;
    GroupElement invCellBoost;
};


   
                                                            
   
RelVector reduceError(RelVector v){
    v.local = reduceError(v.local);
    return v;
}

   
                         
                            
                                                      
   
RelVector add(RelVector v1, RelVector v2){
    v1.local = add(v1.local, v2.local);
    return v1;
}

   
                              
                            
                                                      
   
RelVector sub(RelVector v1, RelVector v2){
    v1.local = sub(v1.local, v2.local);
    return v1;
}

   
                                   
                         
                      
   
RelVector multiplyScalar(float s, RelVector v){
    v.local = multiplyScalar(s, v.local);
    return v;
}

   
                                                                                 
                     
                                                        
   
float geomDot(RelVector v1, RelVector v2) {
    return geomDot(v1.local, v2.local);
}

   
                              
   
RelVector geomNormalize(RelVector v){
    v.local = geomNormalize(v.local);
    return v;
}

   
                                   
                                                        
   
RelVector geomMix(RelVector v1, RelVector v2, float a) {
    v1.local = geomMix(v1.local, v2.local, a);
    return v1;
}

   
                                            
   
RelVector negate(RelVector v){
    v.local = negate(v.local);
    return v;
}

   
                                                                     
                                                       
   
RelVector geomReflect(RelVector v, RelVector normal){
    v.local = geomReflect(v.local, normal.local);
    return v;
}


   
                                                                     
                                                       
   
RelVector geomRefract(RelVector v, RelVector normal, float n){
    v.local = geomRefract(v.local, normal.local, n);
    return v;
}

   
                         
                                            
                                                                         
   
RelVector flow(RelVector v, float t) {
    v.local = flow(v.local, t);
    return v;
}

   
                                                                                          
                                                                           
                                                                          
                               
                                                                                              
   
RelVector smallShift(RelVector v, vec3 dp){
    v.local = smallShift(v.local, dp);
    return v;
                                                 
                                                               
}


   
                                                                                                            
                           
                                                   
                                                                                             
                                            
   
                                                   
                                                   
               
                                                             
                                                                 
   

   
                                                                                                            
                                                   
                                                                                             
   
RelVector createRelVector(RelVector v, vec3 coords){
    v.local =  createVector(v.local.pos, coords);
    return v;
                                                           
                                                               
}

   
                                                                  
                          
                                           
   
RelVector applyPosition(RelPosition position, Vector v) {
    Vector local = applyPosition(position.local, v);
    return RelVector(local, position.cellBoost, position.invCellBoost);
}

   
                                                                                                                    
   
RelVector rewrite(RelVector v, GroupElement elt, GroupElement inv){
    v.local = applyGroupElement(elt, v.local);
                                     
                                       
    v.cellBoost = multiply(v.cellBoost, inv);
    v.invCellBoost = multiply(elt, v.invCellBoost);
    return v;
}


                                                                                                                        
  
                    
                                    
                                                                              
                  
                                                      
                                                                                         
  
                                                                                                                        

struct ExtVector {
    RelVector vector;
    VectorData data;
};


ExtVector flow(ExtVector v, float t) {
    v.vector = flow(v.vector, t);
    v.data.lastFlowDist = t;
    v.data.lastBounceDist = v.data.lastBounceDist + t;
    v.data.totalDist  = v.data.totalDist + t;
    return v;
}







//
    bool test0P(Point p){
        vec4 normal = vec4(0, 0.3995932622677322, 0.6465554800246416, -0.5777087639996633);
        return dot(p.coords, normal) > 0.;
    }
    



/**
 * Default creeping function (binary search)
 * @param start starting point of the creeping
 * @param outside vector out of the boundary (obtained from the previous flow, or the previous creeping)
 * @param offset how long we flow after passing the boundary
 */
float creep66b2e738_51f5_45b9_8ca9_9fbb5613244b(ExtVector v, ExtVector outside,  float offset){
    ExtVector try = outside;
    float sIn = 0.;
    float sOut = try.data.lastFlowDist;
    float s;
    for(int i=0; i < 100; i++){
        if(sOut - sIn < offset){
            break;
        }
        s = 0.5 * sIn + 0.5 * sOut;
        try = flow(v,s);
        if(test0P(try.vector.local.pos)){
            sOut = s;
            outside = try;
        } else {
            sIn = s;
        }
    }
    return sOut;
}



uniform GroupElement groupElement_0a7a70b0_8fc1_4396_965f_47f2dea43e92;



//
    bool test0N(Point p){
        vec4 normal = vec4(0, 0.3995932622677322, 0.6465554800246416, 0.5777087639996633);
        return dot(p.coords, normal) < 0.;
    }
    



/**
 * Default creeping function (binary search)
 * @param start starting point of the creeping
 * @param outside vector out of the boundary (obtained from the previous flow, or the previous creeping)
 * @param offset how long we flow after passing the boundary
 */
float creep4ff8e7d2_7b9b_48a1_85b8_c0e657e42488(ExtVector v, ExtVector outside,  float offset){
    ExtVector try = outside;
    float sIn = 0.;
    float sOut = try.data.lastFlowDist;
    float s;
    for(int i=0; i < 100; i++){
        if(sOut - sIn < offset){
            break;
        }
        s = 0.5 * sIn + 0.5 * sOut;
        try = flow(v,s);
        if(test0N(try.vector.local.pos)){
            sOut = s;
            outside = try;
        } else {
            sIn = s;
        }
    }
    return sOut;
}



uniform GroupElement groupElement_e2d666bb_e3f4_4fb7_a44a_a67866fd2f3d;



//
    bool test1P(Point p){
        vec4 normal = vec4(0, 0.3995932622677322, -0.6465554800246416, -0.5777087639996633);
        return dot(p.coords, normal) > 0.;
    }
    



/**
 * Default creeping function (binary search)
 * @param start starting point of the creeping
 * @param outside vector out of the boundary (obtained from the previous flow, or the previous creeping)
 * @param offset how long we flow after passing the boundary
 */
float creep850d05df_6c52_4b04_82ed_e75900fa188b(ExtVector v, ExtVector outside,  float offset){
    ExtVector try = outside;
    float sIn = 0.;
    float sOut = try.data.lastFlowDist;
    float s;
    for(int i=0; i < 100; i++){
        if(sOut - sIn < offset){
            break;
        }
        s = 0.5 * sIn + 0.5 * sOut;
        try = flow(v,s);
        if(test1P(try.vector.local.pos)){
            sOut = s;
            outside = try;
        } else {
            sIn = s;
        }
    }
    return sOut;
}



uniform GroupElement groupElement_45abd76d_95a9_4110_a1f8_1b05f6f7cc1c;



//
    bool test1N(Point p){
        vec4 normal = vec4(0, 0.3995932622677322, -0.6465554800246416, 0.5777087639996633);
        return dot(p.coords, normal) < 0.;
    }
    



/**
 * Default creeping function (binary search)
 * @param start starting point of the creeping
 * @param outside vector out of the boundary (obtained from the previous flow, or the previous creeping)
 * @param offset how long we flow after passing the boundary
 */
float creep8fab7ab7_647f_402b_8098_f53868a02662(ExtVector v, ExtVector outside,  float offset){
    ExtVector try = outside;
    float sIn = 0.;
    float sOut = try.data.lastFlowDist;
    float s;
    for(int i=0; i < 100; i++){
        if(sOut - sIn < offset){
            break;
        }
        s = 0.5 * sIn + 0.5 * sOut;
        try = flow(v,s);
        if(test1N(try.vector.local.pos)){
            sOut = s;
            outside = try;
        } else {
            sIn = s;
        }
    }
    return sOut;
}



uniform GroupElement groupElement_e9e1fde7_5ffe_42c2_9c57_83e6cd0b76f4;



//
    bool test2P(Point p){
        vec4 normal = vec4(0.3995932622677322, 0.6465554800246416, 0, -0.5777087639996633);
        return dot(p.coords, normal) > 0.;
    }
    



/**
 * Default creeping function (binary search)
 * @param start starting point of the creeping
 * @param outside vector out of the boundary (obtained from the previous flow, or the previous creeping)
 * @param offset how long we flow after passing the boundary
 */
float creep2e037530_2e21_41e4_8d5c_db1a5934faac(ExtVector v, ExtVector outside,  float offset){
    ExtVector try = outside;
    float sIn = 0.;
    float sOut = try.data.lastFlowDist;
    float s;
    for(int i=0; i < 100; i++){
        if(sOut - sIn < offset){
            break;
        }
        s = 0.5 * sIn + 0.5 * sOut;
        try = flow(v,s);
        if(test2P(try.vector.local.pos)){
            sOut = s;
            outside = try;
        } else {
            sIn = s;
        }
    }
    return sOut;
}



uniform GroupElement groupElement_b62f0e49_6e0f_4421_a6cf_c0bc9cd1b572;



//
    bool test2N(Point p){
        vec4 normal = vec4(0.3995932622677322, 0.6465554800246416, 0, 0.5777087639996633);
        return dot(p.coords, normal) < 0.;
    }
    



/**
 * Default creeping function (binary search)
 * @param start starting point of the creeping
 * @param outside vector out of the boundary (obtained from the previous flow, or the previous creeping)
 * @param offset how long we flow after passing the boundary
 */
float creepcf8bc3cc_6f38_4f4d_b0fa_3d76ad1af1d6(ExtVector v, ExtVector outside,  float offset){
    ExtVector try = outside;
    float sIn = 0.;
    float sOut = try.data.lastFlowDist;
    float s;
    for(int i=0; i < 100; i++){
        if(sOut - sIn < offset){
            break;
        }
        s = 0.5 * sIn + 0.5 * sOut;
        try = flow(v,s);
        if(test2N(try.vector.local.pos)){
            sOut = s;
            outside = try;
        } else {
            sIn = s;
        }
    }
    return sOut;
}



uniform GroupElement groupElement_c2f79e0b_dddd_41c9_aed8_f068b5a17af1;



//
    bool test3P(Point p){
        vec4 normal = vec4(0.3995932622677322, -0.6465554800246416, 0, -0.5777087639996633);
        return dot(p.coords, normal) > 0.;
    }
    



/**
 * Default creeping function (binary search)
 * @param start starting point of the creeping
 * @param outside vector out of the boundary (obtained from the previous flow, or the previous creeping)
 * @param offset how long we flow after passing the boundary
 */
float creep8dd69a59_ccb9_4809_8fce_8478abfdef57(ExtVector v, ExtVector outside,  float offset){
    ExtVector try = outside;
    float sIn = 0.;
    float sOut = try.data.lastFlowDist;
    float s;
    for(int i=0; i < 100; i++){
        if(sOut - sIn < offset){
            break;
        }
        s = 0.5 * sIn + 0.5 * sOut;
        try = flow(v,s);
        if(test3P(try.vector.local.pos)){
            sOut = s;
            outside = try;
        } else {
            sIn = s;
        }
    }
    return sOut;
}



uniform GroupElement groupElement_24f9ec79_c6cb_4638_87e4_6a0da4db0cbb;



//
    bool test3N(Point p){
        vec4 normal = vec4(0.3995932622677322, -0.6465554800246416, 0, 0.5777087639996633);
        return dot(p.coords, normal) < 0.;
    }
    



/**
 * Default creeping function (binary search)
 * @param start starting point of the creeping
 * @param outside vector out of the boundary (obtained from the previous flow, or the previous creeping)
 * @param offset how long we flow after passing the boundary
 */
float creep7a6e22e6_7092_4408_b48c_7ae126ee341e(ExtVector v, ExtVector outside,  float offset){
    ExtVector try = outside;
    float sIn = 0.;
    float sOut = try.data.lastFlowDist;
    float s;
    for(int i=0; i < 100; i++){
        if(sOut - sIn < offset){
            break;
        }
        s = 0.5 * sIn + 0.5 * sOut;
        try = flow(v,s);
        if(test3N(try.vector.local.pos)){
            sOut = s;
            outside = try;
        } else {
            sIn = s;
        }
    }
    return sOut;
}



uniform GroupElement groupElement_9daacee9_7cfb_4ffb_aeea_9ec46e30fb87;



//
    bool test4P(Point p){
        vec4 normal = vec4(0.6465554800246416, 0, 0.3995932622677322, -0.5777087639996633);
        return dot(p.coords, normal) > 0.;
    }
    



/**
 * Default creeping function (binary search)
 * @param start starting point of the creeping
 * @param outside vector out of the boundary (obtained from the previous flow, or the previous creeping)
 * @param offset how long we flow after passing the boundary
 */
float creep01e2eaf8_89f2_4cab_80fe_f4a4ab523f41(ExtVector v, ExtVector outside,  float offset){
    ExtVector try = outside;
    float sIn = 0.;
    float sOut = try.data.lastFlowDist;
    float s;
    for(int i=0; i < 100; i++){
        if(sOut - sIn < offset){
            break;
        }
        s = 0.5 * sIn + 0.5 * sOut;
        try = flow(v,s);
        if(test4P(try.vector.local.pos)){
            sOut = s;
            outside = try;
        } else {
            sIn = s;
        }
    }
    return sOut;
}



uniform GroupElement groupElement_7b520eca_90d1_4b98_805c_010dd020e24c;



//
    bool test4N(Point p){
        vec4 normal = vec4(0.6465554800246416, 0, 0.3995932622677322, 0.5777087639996633);
        return dot(p.coords, normal) < 0.;
    }
    



/**
 * Default creeping function (binary search)
 * @param start starting point of the creeping
 * @param outside vector out of the boundary (obtained from the previous flow, or the previous creeping)
 * @param offset how long we flow after passing the boundary
 */
float creep2db63edc_dcbb_48af_bcca_50fd623b6c5d(ExtVector v, ExtVector outside,  float offset){
    ExtVector try = outside;
    float sIn = 0.;
    float sOut = try.data.lastFlowDist;
    float s;
    for(int i=0; i < 100; i++){
        if(sOut - sIn < offset){
            break;
        }
        s = 0.5 * sIn + 0.5 * sOut;
        try = flow(v,s);
        if(test4N(try.vector.local.pos)){
            sOut = s;
            outside = try;
        } else {
            sIn = s;
        }
    }
    return sOut;
}



uniform GroupElement groupElement_4b2e32ff_cf68_4349_b44c_4f240438cc41;



//
    bool test5P(Point p){
        vec4 normal = vec4(-0.6465554800246416, 0, 0.3995932622677322, -0.5777087639996633);
        return dot(p.coords, normal) > 0.;
    }
    



/**
 * Default creeping function (binary search)
 * @param start starting point of the creeping
 * @param outside vector out of the boundary (obtained from the previous flow, or the previous creeping)
 * @param offset how long we flow after passing the boundary
 */
float creepbb65087e_5588_4a9c_8085_663b2fb5c551(ExtVector v, ExtVector outside,  float offset){
    ExtVector try = outside;
    float sIn = 0.;
    float sOut = try.data.lastFlowDist;
    float s;
    for(int i=0; i < 100; i++){
        if(sOut - sIn < offset){
            break;
        }
        s = 0.5 * sIn + 0.5 * sOut;
        try = flow(v,s);
        if(test5P(try.vector.local.pos)){
            sOut = s;
            outside = try;
        } else {
            sIn = s;
        }
    }
    return sOut;
}



uniform GroupElement groupElement_ad2a3b9f_c049_46cd_86f4_d7762fc8ecc5;



//
    bool test5N(Point p){
        vec4 normal = vec4(-0.6465554800246416, 0, 0.3995932622677322, 0.5777087639996633);
        return dot(p.coords, normal) < 0.;
    }
    



/**
 * Default creeping function (binary search)
 * @param start starting point of the creeping
 * @param outside vector out of the boundary (obtained from the previous flow, or the previous creeping)
 * @param offset how long we flow after passing the boundary
 */
float creepab6f98d8_6943_4b33_b318_0e10ad937f06(ExtVector v, ExtVector outside,  float offset){
    ExtVector try = outside;
    float sIn = 0.;
    float sOut = try.data.lastFlowDist;
    float s;
    for(int i=0; i < 100; i++){
        if(sOut - sIn < offset){
            break;
        }
        s = 0.5 * sIn + 0.5 * sOut;
        try = flow(v,s);
        if(test5N(try.vector.local.pos)){
            sOut = s;
            outside = try;
        } else {
            sIn = s;
        }
    }
    return sOut;
}



uniform GroupElement groupElement_eaea16e9_3c3d_4a2a_a227_0b23a2e0b993;



/**
* Teleportation.
* Check if the local vector is still in the fundamental domain define by the teleportation tests.
* If not, teleport the local vector, update the cellBoost and its inverse accordingly and set teleported to true
* Otherwise, do nothing and set teleported to false
* @param[in] v the relative vector to teleport.
*/
ExtVector teleport(ExtVector v){
    v.data.isTeleported = false;
        if(test0P(v.vector.local.pos)){
            v.vector = rewrite(v.vector, groupElement_0a7a70b0_8fc1_4396_965f_47f2dea43e92, groupElement_e2d666bb_e3f4_4fb7_a44a_a67866fd2f3d);
            v.data.isTeleported = true;
            return v;
        }
        if(test0N(v.vector.local.pos)){
            v.vector = rewrite(v.vector, groupElement_e2d666bb_e3f4_4fb7_a44a_a67866fd2f3d, groupElement_0a7a70b0_8fc1_4396_965f_47f2dea43e92);
            v.data.isTeleported = true;
            return v;
        }
        if(test1P(v.vector.local.pos)){
            v.vector = rewrite(v.vector, groupElement_45abd76d_95a9_4110_a1f8_1b05f6f7cc1c, groupElement_e9e1fde7_5ffe_42c2_9c57_83e6cd0b76f4);
            v.data.isTeleported = true;
            return v;
        }
        if(test1N(v.vector.local.pos)){
            v.vector = rewrite(v.vector, groupElement_e9e1fde7_5ffe_42c2_9c57_83e6cd0b76f4, groupElement_45abd76d_95a9_4110_a1f8_1b05f6f7cc1c);
            v.data.isTeleported = true;
            return v;
        }
        if(test2P(v.vector.local.pos)){
            v.vector = rewrite(v.vector, groupElement_b62f0e49_6e0f_4421_a6cf_c0bc9cd1b572, groupElement_c2f79e0b_dddd_41c9_aed8_f068b5a17af1);
            v.data.isTeleported = true;
            return v;
        }
        if(test2N(v.vector.local.pos)){
            v.vector = rewrite(v.vector, groupElement_c2f79e0b_dddd_41c9_aed8_f068b5a17af1, groupElement_b62f0e49_6e0f_4421_a6cf_c0bc9cd1b572);
            v.data.isTeleported = true;
            return v;
        }
        if(test3P(v.vector.local.pos)){
            v.vector = rewrite(v.vector, groupElement_24f9ec79_c6cb_4638_87e4_6a0da4db0cbb, groupElement_9daacee9_7cfb_4ffb_aeea_9ec46e30fb87);
            v.data.isTeleported = true;
            return v;
        }
        if(test3N(v.vector.local.pos)){
            v.vector = rewrite(v.vector, groupElement_9daacee9_7cfb_4ffb_aeea_9ec46e30fb87, groupElement_24f9ec79_c6cb_4638_87e4_6a0da4db0cbb);
            v.data.isTeleported = true;
            return v;
        }
        if(test4P(v.vector.local.pos)){
            v.vector = rewrite(v.vector, groupElement_7b520eca_90d1_4b98_805c_010dd020e24c, groupElement_4b2e32ff_cf68_4349_b44c_4f240438cc41);
            v.data.isTeleported = true;
            return v;
        }
        if(test4N(v.vector.local.pos)){
            v.vector = rewrite(v.vector, groupElement_4b2e32ff_cf68_4349_b44c_4f240438cc41, groupElement_7b520eca_90d1_4b98_805c_010dd020e24c);
            v.data.isTeleported = true;
            return v;
        }
        if(test5P(v.vector.local.pos)){
            v.vector = rewrite(v.vector, groupElement_ad2a3b9f_c049_46cd_86f4_d7762fc8ecc5, groupElement_eaea16e9_3c3d_4a2a_a227_0b23a2e0b993);
            v.data.isTeleported = true;
            return v;
        }
        if(test5N(v.vector.local.pos)){
            v.vector = rewrite(v.vector, groupElement_eaea16e9_3c3d_4a2a_a227_0b23a2e0b993, groupElement_ad2a3b9f_c049_46cd_86f4_d7762fc8ecc5);
            v.data.isTeleported = true;
            return v;
        }
    return v;
}


/**
* Does one of the two following transformation:
* flow the vector by the given time, if the vector escape the fundamental domain,
* then try to find a smaller time so that the vector is moved closer to the boundary of the fudamental domain
* (and even a bit further)
*
* @param[inout] v the relative vector to flow / teleport / creep.
* @param[in] t the (maximal) time to flow
* @param[in] offset the amount we march passed the boundary
*/
float creepingDist(ExtVector v, float t, float offset){
    float res = t;
    ExtVector try = flow(v, t);


            if(test0P(try.vector.local.pos)){
                res = min(res, creep66b2e738_51f5_45b9_8ca9_9fbb5613244b(v, try, offset));
            }



            if(test0N(try.vector.local.pos)){
                res = min(res, creep4ff8e7d2_7b9b_48a1_85b8_c0e657e42488(v, try, offset));
            }



            if(test1P(try.vector.local.pos)){
                res = min(res, creep850d05df_6c52_4b04_82ed_e75900fa188b(v, try, offset));
            }



            if(test1N(try.vector.local.pos)){
                res = min(res, creep8fab7ab7_647f_402b_8098_f53868a02662(v, try, offset));
            }



            if(test2P(try.vector.local.pos)){
                res = min(res, creep2e037530_2e21_41e4_8d5c_db1a5934faac(v, try, offset));
            }



            if(test2N(try.vector.local.pos)){
                res = min(res, creepcf8bc3cc_6f38_4f4d_b0fa_3d76ad1af1d6(v, try, offset));
            }



            if(test3P(try.vector.local.pos)){
                res = min(res, creep8dd69a59_ccb9_4809_8fce_8478abfdef57(v, try, offset));
            }



            if(test3N(try.vector.local.pos)){
                res = min(res, creep7a6e22e6_7092_4408_b48c_7ae126ee341e(v, try, offset));
            }



            if(test4P(try.vector.local.pos)){
                res = min(res, creep01e2eaf8_89f2_4cab_80fe_f4a4ab523f41(v, try, offset));
            }



            if(test4N(try.vector.local.pos)){
                res = min(res, creep2db63edc_dcbb_48af_bcca_50fd623b6c5d(v, try, offset));
            }



            if(test5P(try.vector.local.pos)){
                res = min(res, creepbb65087e_5588_4a9c_8085_663b2fb5c551(v, try, offset));
            }



            if(test5N(try.vector.local.pos)){
                res = min(res, creepab6f98d8_6943_4b33_b318_0e10ad937f06(v, try, offset));
            }

    return res;
}






                                                                                                                        
          
          
                                                                                               
                                                                                                                        
struct Camera {
    float fov;                              
    float minDist;                                     
    float maxDist;                                     
    int maxSteps;                                                       
    float safetyDist;                                                                               
    float threshold;                                          
    RelPosition position;                                                                            
    mat4 matrix;                                                 
};



uniform Camera camera;



   
                                                     
                                                               
   
RelVector mapping(vec3 coords){
    vec4 dir = normalize(vec4(coords, 0));
    dir = normalize(dir);
    Vector v = createVector(ORIGIN, dir.xyz);
    return applyPosition(camera.position, v);
}

                     

  
                               
                               
                         
                                             
                                                      
                              
 
  



                                                                                                                        
                        
                                                                                                                        

struct SingleColorMaterial {
    vec3 color;
};

vec4 render(SingleColorMaterial material, ExtVector v) {
    return vec4(material.color, 1);
}



uniform SingleColorMaterial SingleColorMaterial_8a30fc85_edc5_4be0_9933_1b197c5a6656;



vec4 SingleColorMaterial_8a30fc85_edc5_4be0_9933_1b197c5a6656_render(ExtVector v) {
    return render(SingleColorMaterial_8a30fc85_edc5_4be0_9933_1b197c5a6656, v);
}




   
                              
   
float dist(Point p1, Point p2){
    return acosh(-hypDot(p1.coords, p2.coords));
}



Vector direction(Point p, Point q){
    vec4 dir;
    float c = hypDot(p.coords, q.coords);
    dir = q.coords + c * p.coords;
    dir = dir / sqrt(c * c  - 1.);
    Vector res = Vector(p, dir);
    return res;
}




   
                                                                           
                                                  
                                        
                                
  
float lightIntensity(float len){
                      
    return 1. / (len * len);
                           
}



                                                                                                                        
          
                         
                                                                                                                        

struct PointLight {
    int id;
    Point position;
    vec3 color;
    float intensity;
    int maxDirs;
};

bool directions(PointLight light, RelVector v, int i, out RelVector dir, out float intensity) {
    if (i!=0){
        return false;
    }
    Point position = applyGroupElement(v.invCellBoost, light.position);
    float dist = dist(v.local.pos, position);
    intensity = lightIntensity(dist) * light.intensity;
    Vector local = direction(v.local.pos, position);
    dir = RelVector(local, v.cellBoost, v.invCellBoost);
    return true;
}



uniform PointLight PointLight_14c04fbc_b014_48ed_a8b0_7a46bb79f7ed;



bool PointLight_14c04fbc_b014_48ed_a8b0_7a46bb79f7ed_directions(RelVector v, int i, out RelVector dir, out float intensity) {
    return directions(PointLight_14c04fbc_b014_48ed_a8b0_7a46bb79f7ed, v, i, dir, intensity);
}



uniform PointLight PointLight_f14fa58b_98ab_4a06_ab48_8393284903da;



bool PointLight_f14fa58b_98ab_4a06_ab48_8393284903da_directions(RelVector v, int i, out RelVector dir, out float intensity) {
    return directions(PointLight_f14fa58b_98ab_4a06_ab48_8393284903da, v, i, dir, intensity);
}



uniform PointLight PointLight_4bd131be_f4da_49b7_aa9b_2ee68b8a2186;



bool PointLight_4bd131be_f4da_49b7_aa9b_2ee68b8a2186_directions(RelVector v, int i, out RelVector dir, out float intensity) {
    return directions(PointLight_4bd131be_f4da_49b7_aa9b_2ee68b8a2186, v, i, dir, intensity);
}



uniform PointLight PointLight_61dc45ca_2e0b_461c_8943_09943b91f3ce;



bool PointLight_61dc45ca_2e0b_461c_8943_09943b91f3ce_directions(RelVector v, int i, out RelVector dir, out float intensity) {
    return directions(PointLight_61dc45ca_2e0b_461c_8943_09943b91f3ce, v, i, dir, intensity);
}



                                                                                                                        
          
                             
                                                                                                                        

struct LocalCylinderShape {
    int id;
    Vector direction;
    float radius;
    Isometry absoluteIsomInv;
};

   
                                                    
   
float sdf(LocalCylinderShape cyl, RelVector v) {
    float aux0 = hypDot(v.local.pos.coords, cyl.direction.pos.coords);
    float aux1 = hypDot(v.local.pos.coords, cyl.direction.dir);
    return acosh(sqrt(aux0 * aux0 - aux1 * aux1)) - cyl.radius;
}

   
                                                 
   
RelVector gradient(LocalCylinderShape cyl, RelVector v){
    float aux0 = hypDot(v.local.pos.coords, cyl.direction.pos.coords);
    float aux1 = hypDot(v.local.pos.coords, cyl.direction.dir);
    vec4 coords = - aux0 * cyl.direction.pos.coords + aux1 * cyl.direction.dir;
    Point proj = Point(hypNormalize(coords));
    Vector local = direction(v.local.pos, proj);
    local = negate(local);
    return RelVector(local, v.cellBoost, v.invCellBoost);
}

   
         
                                                                                                              
                                                                                
                                 
   
                                                       
   




uniform LocalCylinderShape LocalCylinderShape_81515c41_e9d4_436f_8ba9_c36d39fe468a;



float LocalCylinderShape_81515c41_e9d4_436f_8ba9_c36d39fe468a_sdf(RelVector v) {
    return sdf(LocalCylinderShape_81515c41_e9d4_436f_8ba9_c36d39fe468a,v);
}


RelVector LocalCylinderShape_81515c41_e9d4_436f_8ba9_c36d39fe468a_gradient(RelVector v){
    return gradient(LocalCylinderShape_81515c41_e9d4_436f_8ba9_c36d39fe468a,v);
}




                                                                                                                        
                         
                                                                                                                        

struct VaryingColorMaterial {
    vec3 mainColor;
    vec3 weight;
};

vec4 render(VaryingColorMaterial material, ExtVector v) {
    vec3 color = material.mainColor + material.weight * v.vector.local.pos.coords.xyz;
    return vec4(color, 1);
}



uniform VaryingColorMaterial VaryingColorMaterial_1612a2f5_83e9_4e9a_ad36_b6a157494043;



vec4 VaryingColorMaterial_1612a2f5_83e9_4e9a_ad36_b6a157494043_render(ExtVector v) {
    return render(VaryingColorMaterial_1612a2f5_83e9_4e9a_ad36_b6a157494043, v);
}




                                                                                                                        
          
                      
                                                                                                                        

struct PhongWrapMaterial {
    float ambient;
    float diffuse;
    float specular;
    float shininess;
    vec3 reflectivity;
};

vec3 lightComputation(Vector v, Vector n, Vector dir, vec3 baseColor, PhongWrapMaterial material, vec3 lightColor, float intensity){
    Vector auxV = negate(v);
    Vector auxL = dir;
    Vector auxN = n;
    Vector auxR = geomReflect(negate(auxL), auxN);
    float NdotL = max(geomDot(auxN, auxL), 0.);
    float RdotV = max(geomDot(auxR, auxV), 0.);

                                                 
    float specularCoeff = material.specular * pow(RdotV, material.shininess);
    float diffuseCoeff = material.diffuse * NdotL;
    float ambientCoeff = material.ambient;

                                                
                                                                                                                                                                                                                                      
    vec3 specularLight = lightColor.rgb;
    vec3 diffuseLight = 0.8 * lightColor.rgb + 0.2 * vec3(1.);
    vec3 ambientLight = 0.5 * lightColor.rgb + 0.5 * vec3(1.);

                                                 
    vec3 specular = specularCoeff * specularLight;
    vec3 diffuse = diffuseCoeff * diffuseLight * baseColor;
    vec3 ambient = ambientCoeff * ambientLight * baseColor;

                 
    vec3 res = intensity * (ambient + diffuse + specular);

    return res;
}



uniform PhongWrapMaterial PhongWrapMaterial_f4a4cd09_a5c6_431a_a9d2_9d895e21de20;



vec4 PhongWrapMaterial_f4a4cd09_a5c6_431a_a9d2_9d895e21de20_render(ExtVector v, RelVector normal) {
    bool check;
    RelVector dir;
    float intensity;
    int k;

    PhongWrapMaterial material = PhongWrapMaterial_f4a4cd09_a5c6_431a_a9d2_9d895e21de20;
    vec3 baseColor = VaryingColorMaterial_1612a2f5_83e9_4e9a_ad36_b6a157494043_render(v).rgb;
    vec3 color = vec3(0);

        k = PointLight_14c04fbc_b014_48ed_a8b0_7a46bb79f7ed.maxDirs;
        for(int j=0; j < k; j++){
            check = PointLight_14c04fbc_b014_48ed_a8b0_7a46bb79f7ed_directions(v.vector, j, dir, intensity);
            if(check) {
                color = color + lightComputation(v.vector.local, normal.local, dir.local, baseColor, material, PointLight_14c04fbc_b014_48ed_a8b0_7a46bb79f7ed.color, intensity);
            }
        }
        k = PointLight_f14fa58b_98ab_4a06_ab48_8393284903da.maxDirs;
        for(int j=0; j < k; j++){
            check = PointLight_f14fa58b_98ab_4a06_ab48_8393284903da_directions(v.vector, j, dir, intensity);
            if(check) {
                color = color + lightComputation(v.vector.local, normal.local, dir.local, baseColor, material, PointLight_f14fa58b_98ab_4a06_ab48_8393284903da.color, intensity);
            }
        }
        k = PointLight_4bd131be_f4da_49b7_aa9b_2ee68b8a2186.maxDirs;
        for(int j=0; j < k; j++){
            check = PointLight_4bd131be_f4da_49b7_aa9b_2ee68b8a2186_directions(v.vector, j, dir, intensity);
            if(check) {
                color = color + lightComputation(v.vector.local, normal.local, dir.local, baseColor, material, PointLight_4bd131be_f4da_49b7_aa9b_2ee68b8a2186.color, intensity);
            }
        }
        k = PointLight_61dc45ca_2e0b_461c_8943_09943b91f3ce.maxDirs;
        for(int j=0; j < k; j++){
            check = PointLight_61dc45ca_2e0b_461c_8943_09943b91f3ce_directions(v.vector, j, dir, intensity);
            if(check) {
                color = color + lightComputation(v.vector.local, normal.local, dir.local, baseColor, material, PointLight_61dc45ca_2e0b_461c_8943_09943b91f3ce.color, intensity);
            }
        }
    
    return vec4(color, 1);
}



                                                                                                                        
  
           
                               
  
                                                                                                                        
 
 struct Solid {
    bool isRendered;
 };








uniform Solid LocalCylinder_7ae8b280_f591_4ad8_a591_e01b20ff2b3e;




            bool LocalCylinder_7ae8b280_f591_4ad8_a591_e01b20ff2b3e_isRenderedHack = true;
        



uniform LocalCylinderShape LocalCylinderShape_94da07d2_a39d_43e3_a56e_9fffbaeb5e49;



float LocalCylinderShape_94da07d2_a39d_43e3_a56e_9fffbaeb5e49_sdf(RelVector v) {
    return sdf(LocalCylinderShape_94da07d2_a39d_43e3_a56e_9fffbaeb5e49,v);
}


RelVector LocalCylinderShape_94da07d2_a39d_43e3_a56e_9fffbaeb5e49_gradient(RelVector v){
    return gradient(LocalCylinderShape_94da07d2_a39d_43e3_a56e_9fffbaeb5e49,v);
}




uniform Solid LocalCylinder_62949112_e7a2_42a6_a0b9_4984ebe0ab5e;




            bool LocalCylinder_62949112_e7a2_42a6_a0b9_4984ebe0ab5e_isRenderedHack = true;
        



uniform LocalCylinderShape LocalCylinderShape_ed64300b_217a_4220_bfc1_835b9545fa2f;



float LocalCylinderShape_ed64300b_217a_4220_bfc1_835b9545fa2f_sdf(RelVector v) {
    return sdf(LocalCylinderShape_ed64300b_217a_4220_bfc1_835b9545fa2f,v);
}


RelVector LocalCylinderShape_ed64300b_217a_4220_bfc1_835b9545fa2f_gradient(RelVector v){
    return gradient(LocalCylinderShape_ed64300b_217a_4220_bfc1_835b9545fa2f,v);
}




uniform Solid LocalCylinder_0b1da693_8372_4d2b_9521_fe6aa50845fd;




            bool LocalCylinder_0b1da693_8372_4d2b_9521_fe6aa50845fd_isRenderedHack = true;
        



uniform LocalCylinderShape LocalCylinderShape_627fdfe9_9db2_4dbd_9298_53a2a0b40853;



float LocalCylinderShape_627fdfe9_9db2_4dbd_9298_53a2a0b40853_sdf(RelVector v) {
    return sdf(LocalCylinderShape_627fdfe9_9db2_4dbd_9298_53a2a0b40853,v);
}


RelVector LocalCylinderShape_627fdfe9_9db2_4dbd_9298_53a2a0b40853_gradient(RelVector v){
    return gradient(LocalCylinderShape_627fdfe9_9db2_4dbd_9298_53a2a0b40853,v);
}




uniform Solid LocalCylinder_5e54e93d_f029_479a_bb6e_53283d812485;




            bool LocalCylinder_5e54e93d_f029_479a_bb6e_53283d812485_isRenderedHack = true;
        



uniform LocalCylinderShape LocalCylinderShape_ffb4901b_ad26_4938_ad0b_549e42b988bc;



float LocalCylinderShape_ffb4901b_ad26_4938_ad0b_549e42b988bc_sdf(RelVector v) {
    return sdf(LocalCylinderShape_ffb4901b_ad26_4938_ad0b_549e42b988bc,v);
}


RelVector LocalCylinderShape_ffb4901b_ad26_4938_ad0b_549e42b988bc_gradient(RelVector v){
    return gradient(LocalCylinderShape_ffb4901b_ad26_4938_ad0b_549e42b988bc,v);
}




uniform Solid LocalCylinder_bddaec34_0dee_4858_9984_488f4171150b;




            bool LocalCylinder_bddaec34_0dee_4858_9984_488f4171150b_isRenderedHack = true;
        



uniform LocalCylinderShape LocalCylinderShape_0830ec29_d025_4942_b322_f30e3cad3bc5;



float LocalCylinderShape_0830ec29_d025_4942_b322_f30e3cad3bc5_sdf(RelVector v) {
    return sdf(LocalCylinderShape_0830ec29_d025_4942_b322_f30e3cad3bc5,v);
}


RelVector LocalCylinderShape_0830ec29_d025_4942_b322_f30e3cad3bc5_gradient(RelVector v){
    return gradient(LocalCylinderShape_0830ec29_d025_4942_b322_f30e3cad3bc5,v);
}




uniform Solid LocalCylinder_755bb12b_0c5e_4784_a1a4_10cf234cf141;




            bool LocalCylinder_755bb12b_0c5e_4784_a1a4_10cf234cf141_isRenderedHack = true;
        



                                                                                                                        
          
                             
                                                                                                                        

struct LocalBallShape {
    int id;
    Point center;
    float radius;
};

   
                                                 
   
float sdf(LocalBallShape ball, RelVector v) {
    return dist(v.local.pos, ball.center) - ball.radius;
}

   
                                              
   
RelVector gradient(LocalBallShape ball, RelVector v){
    Vector local = direction(v.local.pos, ball.center);
    return RelVector(negate(local), v.cellBoost, v.invCellBoost);
}

vec2 uvMap(LocalBallShape ball, RelVector v){
    Vector radius = direction(ball.center, v.local.pos);
    Vector[3] f;
    orthoFrame(v.local.pos,f);
    float x = geomDot(radius, f[0]);
    float y = geomDot(radius, f[1]);
    float cosPhi = geomDot(radius, f[2]);
    float sinPhi = sqrt(x * x + y * y);
    float uCoord = atan(y, x);
    float vCoord = atan(sinPhi, cosPhi);
    return vec2(uCoord, vCoord);
}




uniform LocalBallShape LocalBallShape_3ce54644_c5a5_4395_9d7d_487632a9b4ed;



float LocalBallShape_3ce54644_c5a5_4395_9d7d_487632a9b4ed_sdf(RelVector v) {
    return sdf(LocalBallShape_3ce54644_c5a5_4395_9d7d_487632a9b4ed,v);
}


RelVector LocalBallShape_3ce54644_c5a5_4395_9d7d_487632a9b4ed_gradient(RelVector v){
    return gradient(LocalBallShape_3ce54644_c5a5_4395_9d7d_487632a9b4ed,v);
}


vec2 LocalBallShape_3ce54644_c5a5_4395_9d7d_487632a9b4ed_uvMap(RelVector v){
    return uvMap(LocalBallShape_3ce54644_c5a5_4395_9d7d_487632a9b4ed, v);
}



uniform SingleColorMaterial SingleColorMaterial_3061b8b3_8af3_4365_ae10_944d6629a9a0;



vec4 SingleColorMaterial_3061b8b3_8af3_4365_ae10_944d6629a9a0_render(ExtVector v) {
    return render(SingleColorMaterial_3061b8b3_8af3_4365_ae10_944d6629a9a0, v);
}








uniform Solid LocalBall_fb549aa5_c396_431e_a15c_79ff2f5fa781;




            bool LocalBall_fb549aa5_c396_431e_a15c_79ff2f5fa781_isRenderedHack = true;
        



                                                                                                                        
  
          
                                 
  
                                                                                                                        

struct ExpFog {
    vec3 color;                             
    float scattering;                                                    
};




uniform ExpFog fog;



vec3 applyFog(vec3 color, float dist){
    float coeff = exp(- fog.scattering * dist);
    return coeff * color + (1. - coeff) * fog.color;
}

vec4 applyFog(vec4 color, float dist){
    float coeff = exp(- fog.scattering * dist);
    return coeff * color + (1. - coeff) * vec4(fog.color, 1);
}



/***********************************************************************************************************************
 ***********************************************************************************************************************
 * 
 * Defines the scene SDF and scene Material computations used during the ray-marching and lightening.
 *
 ***********************************************************************************************************************
 **********************************************************************************************************************/

/**
 * Distance along the geodesic directed by \`v\` to the closest object in the local scene
 * @param[in] v the direction to follows
 * @param[out] hit say if we hit an object (1), nothing (0) or if there is a bug (-1)
 * @param[out] objId the ID of the solid we hit.
 */
float _localSceneSDF(RelVector v, out int hit, out int objId){
    hit = HIT_NOTHING;
    float res = camera.maxDist;
    float dist;

            if(LocalCylinder_7ae8b280_f591_4ad8_a591_e01b20ff2b3e.isRendered && LocalCylinder_7ae8b280_f591_4ad8_a591_e01b20ff2b3e_isRenderedHack){
                dist = LocalCylinderShape_81515c41_e9d4_436f_8ba9_c36d39fe468a_sdf(v);
                if(dist < camera.threshold) {
                    hit = HIT_SOLID;
                    objId = 3;
                    return dist;
                }
                res = min(res, dist);
            }
            if(LocalCylinder_62949112_e7a2_42a6_a0b9_4984ebe0ab5e.isRendered && LocalCylinder_62949112_e7a2_42a6_a0b9_4984ebe0ab5e_isRenderedHack){
                dist = LocalCylinderShape_94da07d2_a39d_43e3_a56e_9fffbaeb5e49_sdf(v);
                if(dist < camera.threshold) {
                    hit = HIT_SOLID;
                    objId = 7;
                    return dist;
                }
                res = min(res, dist);
            }
            if(LocalCylinder_0b1da693_8372_4d2b_9521_fe6aa50845fd.isRendered && LocalCylinder_0b1da693_8372_4d2b_9521_fe6aa50845fd_isRenderedHack){
                dist = LocalCylinderShape_ed64300b_217a_4220_bfc1_835b9545fa2f_sdf(v);
                if(dist < camera.threshold) {
                    hit = HIT_SOLID;
                    objId = 11;
                    return dist;
                }
                res = min(res, dist);
            }
            if(LocalCylinder_5e54e93d_f029_479a_bb6e_53283d812485.isRendered && LocalCylinder_5e54e93d_f029_479a_bb6e_53283d812485_isRenderedHack){
                dist = LocalCylinderShape_627fdfe9_9db2_4dbd_9298_53a2a0b40853_sdf(v);
                if(dist < camera.threshold) {
                    hit = HIT_SOLID;
                    objId = 15;
                    return dist;
                }
                res = min(res, dist);
            }
            if(LocalCylinder_bddaec34_0dee_4858_9984_488f4171150b.isRendered && LocalCylinder_bddaec34_0dee_4858_9984_488f4171150b_isRenderedHack){
                dist = LocalCylinderShape_ffb4901b_ad26_4938_ad0b_549e42b988bc_sdf(v);
                if(dist < camera.threshold) {
                    hit = HIT_SOLID;
                    objId = 19;
                    return dist;
                }
                res = min(res, dist);
            }
            if(LocalCylinder_755bb12b_0c5e_4784_a1a4_10cf234cf141.isRendered && LocalCylinder_755bb12b_0c5e_4784_a1a4_10cf234cf141_isRenderedHack){
                dist = LocalCylinderShape_0830ec29_d025_4942_b322_f30e3cad3bc5_sdf(v);
                if(dist < camera.threshold) {
                    hit = HIT_SOLID;
                    objId = 23;
                    return dist;
                }
                res = min(res, dist);
            }
            if(LocalBall_fb549aa5_c396_431e_a15c_79ff2f5fa781.isRendered && LocalBall_fb549aa5_c396_431e_a15c_79ff2f5fa781_isRenderedHack){
                dist = LocalBallShape_3ce54644_c5a5_4395_9d7d_487632a9b4ed_sdf(v);
                if(dist < camera.threshold) {
                    hit = HIT_SOLID;
                    objId = 27;
                    return dist;
                }
                res = min(res, dist);
            }
    
    return res;
}

/**
* Distance along the geodesic directed by \`v\` to the closest object in the local scene
* When nearest neighbor is on, the representation of v can be updated
* so that the local vector is in a neighbor of the fundamental domain.
* This is used to compute correctly the normal / uv map of a local object.
* @param[in] v the direction to follows
* @param[out] hit say if we hit an object (1), nothing (0) or if there is a bug (-1)
* @param[out] objId the ID of the solid we hit.
*/
float localSceneSDF(inout RelVector v, out int hit, out int objId){
    float res, dist;
    dist = _localSceneSDF(v, hit, objId);
    if(hit == HIT_SOLID) {
        return dist;
    }
    res = dist;
    

        return res;
}


/**
 * Distance along the geodesic directed by \`v\` to the closest object in the global scene
 * @param[in] v the direction to follows
 * @param[out] hit say if we hit an object (1), nothing (0) or if there is a bug (-1)
 * @param[out] objID the ID of the solid we hit.
 */
float globalSceneSDF(RelVector v, out int hit, out int objId){
    hit = HIT_NOTHING;
    float res = camera.maxDist;
    float dist;
    
    
    return res;
}



VectorData initVectorData(){
    return VectorData(0., 0., 0., false, 0, 0, false, vec4(0), vec4(1));
}


void updateVectorDataFromSolid(inout ExtVector v, int objId){
    RelVector normal;
    vec2 uv;
    vec4 color;
    vec4 reflectivity;
    float opacity;
    float t;

    switch(objId){

        case 3:




                        normal = LocalCylinderShape_81515c41_e9d4_436f_8ba9_c36d39fe468a_gradient(v.vector);
                        color = PhongWrapMaterial_f4a4cd09_a5c6_431a_a9d2_9d895e21de20_render(v, normal);

                    color = applyFog(color, v.data.lastBounceDist);

                v.data.pixel = v.data.pixel + v.data.leftToComputeColor * color;
                v.data.leftToComputeColor = vec4(0);
                v.data.stop = true;

        break;


        case 7:




                        normal = LocalCylinderShape_94da07d2_a39d_43e3_a56e_9fffbaeb5e49_gradient(v.vector);
                        color = PhongWrapMaterial_f4a4cd09_a5c6_431a_a9d2_9d895e21de20_render(v, normal);

                    color = applyFog(color, v.data.lastBounceDist);

                v.data.pixel = v.data.pixel + v.data.leftToComputeColor * color;
                v.data.leftToComputeColor = vec4(0);
                v.data.stop = true;

        break;


        case 11:




                        normal = LocalCylinderShape_ed64300b_217a_4220_bfc1_835b9545fa2f_gradient(v.vector);
                        color = PhongWrapMaterial_f4a4cd09_a5c6_431a_a9d2_9d895e21de20_render(v, normal);

                    color = applyFog(color, v.data.lastBounceDist);

                v.data.pixel = v.data.pixel + v.data.leftToComputeColor * color;
                v.data.leftToComputeColor = vec4(0);
                v.data.stop = true;

        break;


        case 15:




                        normal = LocalCylinderShape_627fdfe9_9db2_4dbd_9298_53a2a0b40853_gradient(v.vector);
                        color = PhongWrapMaterial_f4a4cd09_a5c6_431a_a9d2_9d895e21de20_render(v, normal);

                    color = applyFog(color, v.data.lastBounceDist);

                v.data.pixel = v.data.pixel + v.data.leftToComputeColor * color;
                v.data.leftToComputeColor = vec4(0);
                v.data.stop = true;

        break;


        case 19:




                        normal = LocalCylinderShape_ffb4901b_ad26_4938_ad0b_549e42b988bc_gradient(v.vector);
                        color = PhongWrapMaterial_f4a4cd09_a5c6_431a_a9d2_9d895e21de20_render(v, normal);

                    color = applyFog(color, v.data.lastBounceDist);

                v.data.pixel = v.data.pixel + v.data.leftToComputeColor * color;
                v.data.leftToComputeColor = vec4(0);
                v.data.stop = true;

        break;


        case 23:




                        normal = LocalCylinderShape_0830ec29_d025_4942_b322_f30e3cad3bc5_gradient(v.vector);
                        color = PhongWrapMaterial_f4a4cd09_a5c6_431a_a9d2_9d895e21de20_render(v, normal);

                    color = applyFog(color, v.data.lastBounceDist);

                v.data.pixel = v.data.pixel + v.data.leftToComputeColor * color;
                v.data.leftToComputeColor = vec4(0);
                v.data.stop = true;

        break;


        case 27:



                        color =  SingleColorMaterial_3061b8b3_8af3_4365_ae10_944d6629a9a0_render(v);


                    color = applyFog(color, v.data.lastBounceDist);

                v.data.pixel = v.data.pixel + v.data.leftToComputeColor * color;
                v.data.leftToComputeColor = vec4(0);
                v.data.stop = true;

        break;

    }
}

void updateVectorData(inout ExtVector v, int hit, int objId){
    if (hit == HIT_DEBUG) {
        v.data.pixel = debugColor;
        v.data.leftToComputeColor = vec4(0);
        v.data.stop = true;
        return;
    }
    if (hit == HIT_NOTHING) {
        vec4 color = SingleColorMaterial_8a30fc85_edc5_4be0_9933_1b197c5a6656_render(v);
        v.data.pixel = v.data.pixel + v.data.leftToComputeColor * color;
        v.data.leftToComputeColor = vec4(0);
        v.data.stop = true;
        return;
    }
    if(hit == HIT_SOLID) {
        updateVectorDataFromSolid(v, objId);
        return;
    }
}

int raymarch(inout ExtVector v, out int objId){
    initFlow(v.vector.local);                                                                 
    ExtVector globalV0 = v;
    ExtVector globalV = globalV0;
    ExtVector localV0 = v;
    ExtVector localV = localV0;
    ExtVector res = v;
    int auxId;
    int auxHit;
    float marchingStep = camera.minDist;
    float dist;
    int hit = HIT_NOTHING;

    for (int i = 0; i < camera.maxSteps; i++){
                          
        localV.data.iMarch = v.data.iMarch + i;

        localV = teleport(localV);
        if (localV.data.isTeleported){
                                                                                           
            localV0 = localV;
                                                                                      
            marchingStep = camera.minDist;
        }
        else {
                                                    
            if (localV.data.totalDist > camera.maxDist) {
                break;
            }
            dist = localSceneSDF(localV.vector, auxHit, auxId);
            if (auxHit == HIT_DEBUG){
                hit = HIT_DEBUG;
                break;
            }
            if (auxHit == HIT_SOLID) {
                                   
                hit = HIT_SOLID;
                objId = auxId;
                v = localV;
                break;
            }
            marchingStep = marchingStep + creepingDist(localV, dist, camera.threshold);
            localV = flow(localV0, marchingStep);
        }
    }

                               
                     
       

                  
    marchingStep = camera.minDist;
    for (int i=0; i < camera.maxSteps; i++){
                          
        globalV.data.iMarch = v.data.iMarch + i;

        if (globalV.data.totalDist > localV.data.totalDist || globalV.data.totalDist > camera.maxDist){
                                              
            break;
        }
        dist = globalSceneSDF(globalV.vector, auxHit, auxId);

        if (auxHit == HIT_DEBUG){
            hit = HIT_DEBUG;
            break;
        }
        if (auxHit == HIT_SOLID) {
                               
            hit = auxHit;
            objId = auxId;
            v = globalV;
            break;
        }
        marchingStep = marchingStep + dist;
        globalV = flow(globalV0, marchingStep);
    }

    if (hit == HIT_NOTHING) {
        v = globalV;
    }
    return hit;
}

vec4 getColor(ExtVector v){
    int objId;
    int hit;
    v = flow(v, camera.safetyDist);
    for (int i = 0; i <= maxBounces; i++){
        if (v.data.stop){
            break;
        }
        hit = raymarch(v, objId);
        updateVectorData(v, hit, objId);
    }
    return v.data.pixel;
}



vec4 postProcess(vec4 color) {
    return color;
}



   
                          
   
varying vec3 screenPosition;

   
                                           
                       
                                                           
                                 
                                                         
   
void main() {
    RelVector vector = mapping(screenPosition);
    ExtVector v = ExtVector(vector, initVectorData());
    gl_FragColor = postProcess(getColor(v));
}