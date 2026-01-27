import { defineConfig } from 'vitepress'

const base = "/webside/"; 
export default defineConfig({
  base, 
  title: "KernelSU on Huawei",
  description: "为你的华为设备吃上内核Root",
  themeConfig: {
    // https://vitepress.dev/reference/default-theme-config
    nav: [
      { text: '主页', link: '/' }
    ],

    sidebar: [
      {
        text: '指导文档',
        items: [
          { text: '项目介绍', link: '/guide/what-is-this.md' },
          { text: '如何使用', link: '/guide/how-to-use.md' },
          { text: '如何适配', link: '/guide/how-to-make.md' },
          { text: '如何构建', link: '/guide/how-to-build.md' }
        ]
      }
    ],

    socialLinks: [
      { icon: 'github', link: 'https://github.com/xixiaobei-bei/KernelSU_on_Huawei' }
    ]
  }
})
