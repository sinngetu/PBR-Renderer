#include <glad/glad.h>
#include <renderer/global.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#include "util.h"

/**
 * 图片导出
 * @param FBO       需导出的 framebuffer
 * @param filename  导出的文件名
 * @param width     图片宽度
 * @param height    图片高度
 * @param channels  通道数量
 * @param format    纹理格式
 * @param outputDir 导出目录
 */
void util::write::image(
    GLuint &FBO, const char *filename,
    GLsizei width, GLsizei height,
    GLsizei channels, GLenum format,
    const char *outputDir
) {
    std::vector<char> buffer(width * height * channels);

    glBindFramebuffer(GL_FRAMEBUFFER, FBO);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, format, GL_UNSIGNED_BYTE, buffer.data());

    auto path = global::resolvePath((std::string(outputDir) + std::string(filename)).c_str());

    stbi_write_png(path.c_str(), width, height, channels, buffer.data(), width * channels);
}

/**
 * cubemap 导出
 * @param FBO        需导出的 framebuffer
 * @param filename   导出的文件名
 * @param resolution 每个面的图片分辨率
 * @param channels   通道数量
 * @param format     纹理格式
 * @param outputDir  导出目录
 */
void util::write::cubemap(GLuint &cubemap,
    GLuint &FBO, const char *filename, GLsizei resolution,
    GLsizei channels, GLenum format, const char *outputDir
) {
    std::vector<char> buffer(resolution * resolution * channels);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    auto prefix = global::resolvePath((std::string(outputDir) + std::string(filename)).c_str());

    for(unsigned int i = 0; i < 6; i++) {
        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap, 0);
        glReadPixels(0, 0, resolution, resolution, format, GL_UNSIGNED_BYTE, buffer.data());

        std::string path = prefix.string();
        std::string index = std::to_string(i + 1);

        index = '_' + index;
        path.replace(path.find(".png"), 0, index.c_str());

        stbi_write_png(path.c_str(), resolution, resolution, channels, buffer.data(), resolution * channels);
    }
}
