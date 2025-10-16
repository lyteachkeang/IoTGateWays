const express = require('express');
const cors = require('cors');
const db = require('./firebase');

const app = express();
app.use(express.json());
app.use(cors());

app.post('/api/data/:gatewayId', async (req, res) => {
  const { gatewayId } = req.params;
  const data = req.body;

  try {
    await db.ref(`sensor_data/${gatewayId}`).push(data);
    console.log(`✅ Data saved for ${gatewayId}:`, data);
    res.json({ status: 'success', message: 'Data saved to Firebase' });
  } catch (err) {
    console.error("❌ Error saving data:", err);
    res.status(500).json({ status: 'error', message: err.message });
  }
});


app.get('/api/data/:gatewayId', async (req, res) => {
  const { gatewayId } = req.params;

  try {
    const snapshot = await db.ref(`sensor_data/${gatewayId}`).once('value');
    const data = snapshot.val();
    res.json(data || {});
  } catch (err) {
    console.error("❌ Error fetching data:", err);
    res.status(500).json({ status: 'error', message: err.message });
  }
});

const PORT = 3000;
app.listen(PORT, () => {
  console.log(`🚀 Backend server running on http://localhost:${PORT}`);
});
