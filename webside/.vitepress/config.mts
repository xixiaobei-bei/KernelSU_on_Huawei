import { defineConfig } from 'vitepress'

export default defineConfig({
  base: '/KernelSU_on_Huawei/',
  srcDir: 'docs',

  locales: {
    root: {
      label: '简体中文',
      lang: 'zh-CN',
      link: '/',
      title: "KernelSU on Huawei",
      description: "为你的华为设备吃上内核Root",
      themeConfig: {
        nav: [{ text: '主页', link: '/' }]
      }
    },
    en: {
      label: 'English',
      lang: 'en-US',
      link: '/en/',
      title: "KernelSU on Huawei",
      description: "Get kernel-level root for your Huawei devices",
      themeConfig: {
        nav: [{ text: 'Home', link: '/en/' }]
      }
    }
  },

  themeConfig: {
    socialLinks: [
      { icon: 'github', link: 'https://github.com/xixiaobei-bei/KernelSU_on_Huawei' }
    ],

    sidebar: {
      '/': [
        {
          text: '指导文档',
          items: [
            { text: '项目介绍', link: '/guide/what-is-this' },
            { text: '如何使用', link: '/guide/how-to-use' },
            { text: '如何适配', link: '/guide/how-to-adapt' },
            { text: '集成不带SuSFS的内核', link: '/guide/how-to-intergrate-kernelsu' },
            { text: '集成带SuSFS和KPM的内核', link: '/guide/how-to-build-susfs-and-kpm' },
            { text: '编译', link: '/guide/how-to-build' }
          ]
        }
      ],
      '/en/': [
        {
          text: 'Documentation',
          items: [
            { text: 'What is this', link: '/en/guide/what-is-this' },
            { text: 'How to use', link: '/en/guide/how-to-use' },
            { text: 'How to adapt', link: '/en/guide/how-to-adapt' },
            { text: 'Interate KernelSU without SuSFS', link: '/en/guide/how-to-intergrate-kernelsu' },
            { text: 'Interate KernelSU with SuSFS and KPM', link: '/en/guide/how-to-build-susfs-and-kpm' },
            { text: 'Build', link: '/en/guide/how-to-build' }
          ]
        }
      ]
    }
  }
})