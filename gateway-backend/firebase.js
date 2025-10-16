const admin = require('firebase-admin');
const serviceAccount = require('./firebase-service-account.json');

admin.initializeApp({
  credential: admin.credential.cert(serviceAccount),
  databaseURL: "https://realtime-database-d5c4c-default-rtdb.asia-southeast1.firebasedatabase.app"
});

const db = admin.database();

module.exports = db;
