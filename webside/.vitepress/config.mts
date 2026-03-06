import { defineConfig } from 'vitepress'

export default defineConfig({
  base: '/KernelSU_on_Huawei/',
  locales: {
    root: {
      label: '简体中文',
      lang: 'zh-CN',
      link: '/docs/',
      title: "KernelSU on Huawei",
      description: "为你的华为设备吃上内核Root",
      themeConfig: {
        nav: [{ text: '主页', link: '/' }]
      }
    },
    en: {
      label: 'English',
      lang: 'en-US',
      link: '/docs/en/',
      title: "KernelSU on Huawei",
      description: "Get kernel-level root for your Huawei devices",
      themeConfig: {
        nav: [{ text: 'Home', link: '/docs/en/' }]
      }
    }
  },

  themeConfig: {
    socialLinks: [
      { icon: 'github', link: 'https://github.com/xixiaobei-bei/KernelSU_on_Huawei' }
    ],

    sidebar: {
      '/docs/': [
        {
          text: '指导文档',
          items: [
            { text: '项目介绍', link: '/docs/guide/what-is-this' },
            { text: '如何使用', link: '/docs/guide/how-to-use' },
            { text: '如何适配', link: '/docs/guide/how-to-adapt' },
            { text: '如何构建', link: '/docs/guide/how-to-build' }
          ]
        }
      ],
      '/docs/en/': [
        {
          text: 'Documentation',
          items: [
            { text: 'What is this', link: '/docs/en/guide/what-is-this' },
            { text: 'How to use', link: '/docs/en/guide/how-to-use' },
            { text: 'How to adapt', link: '/docs/en/guide/how-to-adapt' },
            { text: 'How to build', link: '/docs/en/guide/how-to-build' }
          ]
        }
      ]
    }
  }
})
