#ifndef RENDERMANAGER_H_
#define RENDERMANAGER_H_

class RenderManager {
public:
    ~RenderManager();

    static RenderManager * getInstance();

    void queryGPUCapabilities();
    
    int getNbMaxTextures();

protected:
    RenderManager();

private:
    RenderManager(const RenderManager &);
    RenderManager &operator=(const RenderManager &);

private:
    static RenderManager *m_pInstance;

    int m_maxTextureNb;
};

#endif /* RENDERMANAGER_H_ */
