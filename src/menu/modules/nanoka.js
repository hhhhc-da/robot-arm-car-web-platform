export default {
  path: '/nanoka',
  title: '核心',
  icon: 'database',
  children: [
    {
      path: '/nanoka/index',
      title: '简介',
      icon: 'home'
    },
    {
      path: '/nanoka/main',
      title: '控制台',
      icon: 'window-restore'
    },
    // {
    //   path: '/nanoka/upload',
    //   title: '上传图片',
    //   icon: 'upload'
    // },
    {
      path: '/nanoka/mapc',
      title: '地图分析',
      icon: 'globe'
    },
    {
      path: '/nanoka/analyze',
      title: '小车跟踪',
      icon: 'camera'
    },
    {
      path: '/nanoka/object_detect',
      title: '物体识别',
      icon: 'tasks'
    },
    {
      path: '/nanoka/robotarm_control',
      title: '机械臂',
      icon: 'bug'
    }
  ]
}
