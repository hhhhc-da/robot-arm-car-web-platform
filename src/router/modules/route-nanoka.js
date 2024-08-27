import layoutHeaderAside from '@/layout/header-aside'

// 由于懒加载页面太多的话会造成webpack热更新太慢，所以开发环境不使用懒加载，只有生产环境使用懒加载
const _import = require('@/libs/util.import.' + process.env.NODE_ENV)

const meta = { auth: true }

export default {
  path: '/nanoka',
  name: 'nanoka',
  meta,
  redirect: { name: 'main' },
  component: layoutHeaderAside,
  children: [
    {
      path: 'index',
      name: 'nanoka-index',
      component: _import('nanoka/index'),
      meta: { ...meta, title: '简介' }
    },
    {
      path: 'main',
      name: 'nanoka-main',
      component: _import('nanoka/main'),
      meta: { ...meta, title: '控制台' }
    },
    // {
    //   path: 'upload',
    //   name: 'nanoka-upload',
    //   component: _import('nanoka/upload'),
    //   meta: { ...meta, title: '图片上传' }
    // },
    {
      path: 'mapc',
      name: 'nanoka-mapc',
      component: _import('nanoka/mapc'),
      meta: { ...meta, title: '地图分析' }
    },
    {
      path: 'analyze',
      name: 'nanoka-analyze',
      component: _import('nanoka/analyze'),
      meta: { ...meta, title: '小车跟踪' }
    },
    {
      path: 'object_detect',
      name: 'nanoka-object-detect',
      component: _import('nanoka/object'),
      meta: { ...meta, title: '物体识别' }
    }
  ]
}
