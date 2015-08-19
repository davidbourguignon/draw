#include "texture.h"

Gauss::Gauss() {
}

Gauss::Gauss(const real u, const real s) {
  mu = u;
  sigma = s;
  const real sqrt_2_Pi = Numerics<real>::sqroot(2.0*M_PI);
  A = 1.0/(sigma*sqrt_2_Pi);
  B = -1.0/(2.0*sigma*sigma);
}

Gauss::real Gauss::operator()(const real x) const {
  real tmp = x - mu;
  return A*Numerics<real>::e(B*tmp*tmp);
}

Gamma::Gamma() {
}

Gamma::Gamma(const integer a, const integer t) {
  alpha = a;
  theta = t;
  A = 1.0/(Numerics<integer>::factorial(alpha - 1)*
	   Numerics<real>::power(theta, alpha));
  B = -1.0/theta;
}

Gamma::real Gamma::operator()(const real x) const {
  return A*Numerics<real>::power(x, alpha - 1)*Numerics<real>::e(B*x);
}

/*****************************************************************************/

bool Texture::isPowerOfTwo(const GLint size) const {
  bool is_power_of_two;
  if (size <= 0) {
    is_power_of_two = false;
  }
  else {
    GLint s = 1;
    while (s < size) {
      s *= 2;
    }
    if (s/size == 1) {
      is_power_of_two = true;
    }
    else {
      is_power_of_two = false;
    }
  }
  return is_power_of_two;
}

void Texture::eval1DProbDistrib(const Probability_Distribution_Function& P,
				std::vector<real>& function,
				const int first, const int size) const {
  /*  Probability distribution function in 1D:
   *  
   *     x in [ 0; 1]
   *  or x in [-1; 1] (not exact for even width...)
   *  
   *  P(x) in [ 0; 1]
   */
  int i, j;
  bool first_result = true;
  real min_result = 0.0;
  for (i = 0, j = first; i < function.size(); i++, j++) {
    const real x = static_cast<real>(j)/static_cast<real>(size);
    const real result = P(x);
    if (first_result) {
      min_result = result;
      first_result = false;
    }
    else if (result < min_result) {
      min_result = result;
    }
    function[i] = result;
  }
  real max_result = 0.0;
  for (i = 0; i < function.size(); i++) {
    const real result = function[i] - min_result;
    if (result > max_result) {
      max_result = result;
    }
    function[i] = result;
  }
  for (i = 0; i < function.size(); i++) {
    function[i] /= max_result;
  }
}

void Texture::filterImageBorders(std::vector<GLfloat>& image,
				 const int pixel_size,
				 const int width, const int height) const {
  typedef Vec2<GLfloat> vec2f;
  const int bandwidthi = 100;
  std::vector<real> filter(bandwidthi);
  eval1DProbDistrib(Gauss(0.0, 5.0e+6), filter, 0, bandwidthi); // Magic nb!
  std::map<GLfloat, GLfloat> f;
  const GLfloat bandwidth = static_cast<GLfloat>(filter.size());
  for (GLfloat x = 0.0; x < bandwidth; x += 1.0) {
    f[x] = filter[static_cast<int>(x)];
  }
  const int width_center  = static_cast<int>(width*0.5);
  const int height_center = static_cast<int>(height*0.5);
  const vec2f center(width_center, height_center);
  const GLfloat dist_max = center.norm();
  int index = 0;
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      const GLfloat dist_rel = vec2f(center - vec2f(j, i)).norm()/dist_max;
      const GLfloat f_value = f[Numerics<GLfloat>::rfloor(bandwidth*dist_rel)];
#if defined(TEST_TEXTURE) || defined(ALPHA_TEXTURE)
      for (int k = 0; k < pixel_size; k++, index++) {
	image[index] *= f_value;
      }
#else
      for (int k = 0; k < pixel_size - 1; k++, index++) {}
      image[index] *= f_value; index++;
#endif
    }
  }
}

void Texture::buildMipmaps(const Probability_Distribution_Function& P,
			   const GLint symmetry, const int filter_type) const {
  assert(!dimensions.empty());
  int width = dimensions[0];
  int height;
  if (dimensions.size() == 1) {
    height = 1;
  }
  else if (dimensions.size() == 2) {
    height = dimensions[1];
  }
  int first, size;
  if (symmetry == 1) {
    first = 0;
    size = width;
  }
  else if (symmetry == 2) {
    int w_2 = static_cast<int>(Numerics<real>::rfloor(width*0.5));
    first = - w_2;
    size = w_2;
  }
  std::vector<real> function(width);
  eval1DProbDistrib(P, function, first, size);
  
  int pixel_size;
  
#if TEST_TEXTURE
  
  /* Build image */
  pixel_size = 4;
  std::vector<GLfloat> image(width*pixel_size);
  for (int i = 0; i < function.size(); i++) {
    const real c = function[i];
    GLfloat color;
    if (c < 0.0) {
      color = static_cast<GLfloat>(0.0);
    }
    else if (c > 1.0) {
      color = static_cast<GLfloat>(1.0);
    }
    else {
      color = static_cast<GLfloat>(c);
    }
    for (int j = 0; j < pixel_size; j++) {
      image[i*4 + j] = color;
    }
  }
  
  /* Load texture */
  if (dimensions.size() == 1) {
    GLint error = gluBuild1DMipmaps(GL_TEXTURE_1D, GL_RGBA, width,
                                    GL_RGBA, GL_FLOAT, &image[0]);
    assert(!error);
  }
  else if (dimensions.size() == 2) {
    std::vector<GLfloat> image_copy(image);
    for (int i = 0; i < height - 1; i++) {
      image.insert(image.end(), image_copy.begin(), image_copy.end());
    }
    if (filter_type != NO_FILTER) {
      filterImageBorders(image, pixel_size, width, height);
    }
    GLint error = gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height,
                                    GL_RGBA, GL_FLOAT, &image[0]);
    assert(!error);
  }
  
  /* Write PPM file */
  std::vector<GLubyte> image_ubyte;
  for (int i = 0; i < image.size(); i += 4) {
    const GLfloat c = 255.0*image[i];
    GLubyte color;
    if (c < 0.0) {
      color = static_cast<GLubyte>(0);
    }
    else if (c > 255.0) {
      color = static_cast<GLubyte>(255);
    }
    else {
      color = static_cast<GLubyte>(c);
    }
    for (int j = 0; j < 3; j++) {
      image_ubyte.push_back(color);
    }
  }
  char id_number[10], name[50];
  static int number = 0;
  sprintf(id_number, "%d", number);
  number++;
  strcpy(name, "test_tex");
  strcat(name, id_number);
  strcat(name, ".ppm");
  FILE* file_out;
  file_out = fopen(name, "wb");
  fprintf(file_out, "P6\n%lu %lu\n255\n", width, height);
  fwrite(&image_ubyte[0], sizeof(GLubyte), image_ubyte.size(), file_out);
  fclose(file_out);
  
#else // TEST_TEXTURE
  
#if ALPHA_TEXTURE
  
  /* Build image */
  pixel_size = 1;
  std::vector<GLfloat> image(width*pixel_size);
  for (int i = 0; i < function.size(); i++) {
    const real a = function[i];
    GLfloat alpha;
    if (a < 0.0) {
      alpha = static_cast<GLfloat>(0.0);
    }
    else if (a > 1.0) {
      alpha = static_cast<GLfloat>(1.0);
    }
    else {
      alpha = static_cast<GLfloat>(a);
    }
    image[i] = alpha;
  }
  
  /* Load texture */
  if (dimensions.size() == 1) {
    GLint error = gluBuild1DMipmaps(GL_TEXTURE_1D, GL_ALPHA, width,
                                    GL_ALPHA, GL_FLOAT, &image[0]);
    assert(!error);
  }
  else if (dimensions.size() == 2) {
    std::vector<GLfloat> image_copy(image);
    for (int i = 0; i < height - 1; i++) {
      image.insert(image.end(), image_copy.begin(), image_copy.end());
    }
    if (filter_type != NO_FILTER) {
      filterImageBorders(image, pixel_size, width, height);
    }
    GLint error = gluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA, width, height,
                                    GL_ALPHA, GL_FLOAT, &image[0]);
    assert(!error);
  }
  
#else // ALPHA_TEXTURE
  
  /* Build image */
  pixel_size = 4;
  std::vector<GLfloat> image(width*pixel_size);
  for (int i = 0; i < function.size(); i++) {
    const real a = function[i];
    GLfloat alpha;
    if (a < 0.0) {
      alpha = static_cast<GLfloat>(0.0);
    }
    else if (a > 1.0) {
      alpha = static_cast<GLfloat>(1.0);
    }
    else {
      alpha = static_cast<GLfloat>(a);
    }
    for (int j = 0; j < 3; j++) {
      image[i*pixel_size + j] = static_cast<GLfloat>(1.0);
    }
    image[i*pixel_size + 3] = alpha;
  }
  
  /* Load texture */
  if (dimensions.size() == 1) {
    GLint error = gluBuild1DMipmaps(GL_TEXTURE_1D, GL_RGBA, width,
                                    GL_RGBA, GL_FLOAT, &image[0]);
    assert(!error);
  }
  else if (dimensions.size() == 2) {
    std::vector<GLfloat> image_copy(image);
    for (int i = 0; i < height - 1; i++) {
      image.insert(image.end(), image_copy.begin(), image_copy.end());
    }
    if (filter_type != NO_FILTER) {
      filterImageBorders(image, pixel_size, width, height);
    }
    GLint error = gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height,
                                    GL_RGBA, GL_FLOAT, &image[0]);
    assert(!error);
  }
  
#endif // ALPHA_TEXTURE
  
#endif // TEST_TEXTURE
}

Texture::Texture() {}

Texture::Texture(char* file_name, const int image_format,
		 const int texture_format) {
  /* Load image */
  int pixel_size;
  int width;
  int height;
  std::vector<GLubyte> image;
  GLubyte* picture;
  fflush(stdout);
  if (image_format == SGI_ALPHA) {
    pixel_size = 1;
    picture = read_alpha_texture(file_name, &width, &height);
  }
  if (image_format == SGI_RGB) {
    pixel_size = 3;
    picture = read_rgb_texture(file_name, &width, &height);
  }
  else if (image_format == SGI_RGBA) {
    pixel_size = 4;
    picture = read_rgba_texture(file_name, &width, &height);
  }
  assert(picture != NULL);
  for (int i = 0; i < pixel_size*width*height; i++) {
    image.push_back(picture[i]);
  }
  free(picture);
  dimensions.push_back(width);
  dimensions.push_back(height);
  
  /* Build texture */
  glPushAttrib(GL_TEXTURE_BIT);
  glGenTextures(1, &name);
  glBindTexture(GL_TEXTURE_2D, name);
  if (glIsTexture(name)) {
    GLint error = 0;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		    GL_LINEAR_MIPMAP_LINEAR);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    if (texture_format == ALPHA) {
      error = gluBuild2DMipmaps(GL_TEXTURE_2D, GL_ALPHA, width, height,
				GL_ALPHA, GL_UNSIGNED_BYTE, &image[0]);
    }
    else if (texture_format == RGB) {
      error = gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height,
				GL_RGB, GL_UNSIGNED_BYTE, &image[0]);
    }
    else if (texture_format == RGBA) {
      error = gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height,
				GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
    }
    assert(!error);
  }
  else {
    assert(false);
  }
  glPopAttrib();
}

Texture::Texture(const Probability_Distribution_Function& P,
		 const GLint symmetry, const std::vector<GLsizei>& dims,
		 const int filter_type) {
  assert(symmetry == 1 || symmetry == 2);
  for (int i = 0; i < dims.size(); i++) {
    assert(isPowerOfTwo(dims[i]));
  }
  dimensions = dims;
  
  /* Build texture */
  glPushAttrib(GL_TEXTURE_BIT);
  glGenTextures(1, &name);
  if (dimensions.size() == 1) {
    glBindTexture(GL_TEXTURE_1D, name);
    if (!glIsTexture(name)) {
      assert(false);
    }
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
  }
  else if (dimensions.size() == 2) {
    glBindTexture(GL_TEXTURE_2D, name);
    if (!glIsTexture(name)) {
      assert(false);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		    GL_LINEAR_MIPMAP_LINEAR);
  }
  else {
    assert(false);
  }
  glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  buildMipmaps(P, symmetry, filter_type);
  glPopAttrib();
}
